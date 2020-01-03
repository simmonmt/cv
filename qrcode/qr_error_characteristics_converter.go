package main

import (
	"bufio"
	"flag"
	"fmt"
	"io"
	"log"
	"os"
	"reflect"
	"regexp"
	"strconv"
	"strings"
)

var (
	headerGuardFlag = flag.String("header_guard", "", "header guard")
	inputFlag       = flag.String("input", "", "TSV input file")
	outputFlag      = flag.String("output", "", "Output header file")
)

func lineError(lineNum int, msg string, args []interface{}) error {
	sub := fmt.Sprintf(msg, args...)
	return fmt.Errorf("%d: %s", lineNum, sub)
}

func readInput(path string) ([][]string, error) {
	fp, err := os.Open(path)
	if err != nil {
		return nil, err
	}
	defer fp.Close()

	out := [][]string{}
	scanner := bufio.NewScanner(fp)
	for lineNum := 1; scanner.Scan(); lineNum++ {
		mkError := func(msg string, args ...interface{}) error {
			return lineError(lineNum, msg, args)
		}

		line := scanner.Text()
		if strings.HasPrefix(line, "#") || line == "" {
			out = append(out, []string{})
			continue
		}

		fields := strings.Split(line, "\t")
		if len(fields) != 6 {
			return nil, mkError("expected 6 fields, got %d",
				lineNum, len(fields))
		}

		out = append(out, strings.Split(line, "\t"))
	}
	if err := scanner.Err(); err != nil {
		return nil, err
	}

	return out, nil
}

type ECCLevel int

const (
	ECC_L ECCLevel = iota
	ECC_M
	ECC_Q
	ECC_H
)

func (l ECCLevel) String() string {
	switch l {
	case ECC_L:
		return "L"
	case ECC_M:
		return "M"
	case ECC_Q:
		return "Q"
	case ECC_H:
		return "H"
	default:
		panic("bad level")
	}
}

func (l ECCLevel) CPPString() string {
	switch l {
	case ECC_L:
		return "QRECC_L"
	case ECC_M:
		return "QRECC_M"
	case ECC_Q:
		return "QRECC_Q"
	case ECC_H:
		return "QRECC_H"
	default:
		panic("bad level")
	}
}

func ParseECCLevel(s string) (ECCLevel, error) {
	switch s {
	case "L":
		return ECC_L, nil
	case "M":
		return ECC_M, nil
	case "Q":
		return ECC_Q, nil
	case "H":
		return ECC_H, nil
	default:
		return ECC_L, fmt.Errorf("bad ecc level %v", s)
	}
}

type BlockDesc struct {
	Num          int
	CodeNumTotal int
	CodeNumData  int
}

type ECCDesc struct {
	Level        ECCLevel
	NumCodewords int // Number of ECC codewords
	Blocks       []BlockDesc
}

type VersionDesc struct {
	Version      int
	NumCodewords int // Total number of data+ECC codewords
	ECC          []ECCDesc
}

const (
	IDX_VERSION = iota
	IDX_TOTAL_CODEWORDS
	IDX_ECC_LEVEL
	IDX_NUM_ECC_CODEWORDS
	IDX_NUM_ECC_BLOCKS
	IDX_ECC_CODE
)

func parseECCDesc(line []string) (*ECCDesc, error) {
	eccLevel, err := ParseECCLevel(line[IDX_ECC_LEVEL])
	if err != nil {
		return nil, err
	}

	numCodewords, err := strconv.Atoi(line[IDX_NUM_ECC_CODEWORDS])
	if err != nil {
		return nil, fmt.Errorf("bad num ecc codewords")
	}

	desc := &ECCDesc{
		Level:        eccLevel,
		NumCodewords: numCodewords,
	}

	return desc, nil
}

var (
	codePattern = regexp.MustCompile(`^\(([0-9]+),([0-9]+),([0-9]+)\)$`)
)

func parseBlockDesc(line []string) (*BlockDesc, error) {
	numBlocks, err := strconv.Atoi(line[IDX_NUM_ECC_BLOCKS])
	if err != nil {
		return nil, fmt.Errorf("bad num ecc blocks")
	}

	parts := codePattern.FindStringSubmatch(line[IDX_ECC_CODE])
	if len(parts) == 0 {
		return nil, fmt.Errorf("bad ecc code")
	}

	numTotal, err := strconv.Atoi(parts[1])
	if err != nil {
		return nil, fmt.Errorf("bad total num in ecc code")
	}
	numData, err := strconv.Atoi(parts[2])
	if err != nil {
		return nil, fmt.Errorf("bad num data in ecc code")
	}

	desc := &BlockDesc{
		Num:          numBlocks,
		CodeNumTotal: numTotal,
		CodeNumData:  numData,
	}

	return desc, err
}

func parseInput(in [][]string) ([]*VersionDesc, error) {
	out := []*VersionDesc{}
	var curVers *VersionDesc
	for i, line := range in {
		lineNum := i + 1
		mkError := func(msg string, args ...interface{}) error {
			return lineError(lineNum, msg, args)
		}

		if len(line) == 0 {
			continue
		}

		if line[IDX_VERSION] != "" {
			version, err := strconv.Atoi(line[IDX_VERSION])
			if err != nil {
				return nil, mkError("bad version")
			}

			numCodewords, err := strconv.Atoi(line[IDX_TOTAL_CODEWORDS])
			if err != nil {
				return nil, mkError("bad total num codewords")
			}

			curVers = &VersionDesc{
				Version:      version,
				NumCodewords: numCodewords,
			}
			out = append(out, curVers)
		}

		if curVers == nil {
			return nil, mkError("missing version desc")
		}

		if line[IDX_ECC_LEVEL] != "" {
			eccDesc, err := parseECCDesc(line)
			if err != nil {
				return nil, mkError("bad ecc desc: %v", err)
			}

			curVers.ECC = append(curVers.ECC, *eccDesc)
		}

		if curVers.ECC == nil {
			return nil, mkError("missing version/ecc desc")
		}

		if line[IDX_NUM_ECC_BLOCKS] != "" {
			blockDesc, err := parseBlockDesc(line)
			if err != nil {
				return nil, mkError("bad block desc: %v", err)
			}

			last := &curVers.ECC[len(curVers.ECC)-1]
			last.Blocks = append(last.Blocks, *blockDesc)
		}

	}

	return out, nil
}

func validateDesc(desc *VersionDesc) error {
	// For each ECC level:
	//   Total number of codewords in version desc =
	//     Total number of codewords in ECC code *
	//     Number of blocks
	for _, eccDesc := range desc.ECC {
		derived := 0
		for _, blockDesc := range eccDesc.Blocks {
			derived += blockDesc.Num * blockDesc.CodeNumTotal
		}

		if derived != desc.NumCodewords {
			return fmt.Errorf("version %d: %v: derived total codewords %d != top %d",
				desc.Version, eccDesc.Level, derived, desc.NumCodewords)
		}
	}

	// For each ECC level:
	//   Number of ECC codewords =
	//     Sum(For each block
	//           (Total number of codewords in ECC code -
	//            Number of data codewords in ECC code) * Number of blocks
	for _, eccDesc := range desc.ECC {
		derived := 0
		for _, blockDesc := range eccDesc.Blocks {
			derived += (blockDesc.CodeNumTotal - blockDesc.CodeNumData) *
				blockDesc.Num
		}

		if derived != eccDesc.NumCodewords {
			return fmt.Errorf("version %d: %v: derived ecc codewords %d != top %d",
				desc.Version, eccDesc.Level, derived, eccDesc.NumCodewords)
		}
	}

	expected := []ECCLevel{ECC_L, ECC_M, ECC_Q, ECC_H}
	got := []ECCLevel{}
	for _, eccDesc := range desc.ECC {
		got = append(got, eccDesc.Level)
	}

	if !reflect.DeepEqual(expected, got) {
		return fmt.Errorf("version %d: expected levels %v, got %v",
			desc.Version, expected, got)
	}

	return nil
}

func validateDescs(descs []*VersionDesc) error {
	for _, desc := range descs {
		if err := validateDesc(desc); err != nil {
			return err
		}
	}
	return nil
}

const (
	headerPreamble = `
#ifndef GUARD
#define GUARD 1

#include "qrcode/qr_error_characteristics.impl.h"

static const ErrorCharacteristicsDesc kQRErrorCharacteristics[] = {
`

	headerPostamble = `
};

#endif // GUARD
`
)

func writeString(w io.Writer, s string) error {
	if n, err := w.Write([]byte(s)); err != nil {
		return err
	} else if n != len(s) {
		return fmt.Errorf("short write")
	}
	return nil
}

func writeHeader(descs []*VersionDesc, w io.Writer) error {
	pre := strings.ReplaceAll(headerPreamble, "GUARD", *headerGuardFlag)
	if err := writeString(w, pre); err != nil {
		return err
	}

	for _, desc := range descs {
		str := fmt.Sprintf("{%d, { ", desc.Version)

		for _, eccDesc := range desc.ECC {
			str += "{" // begin the LevelDesc
			str += "{" // begin the block_sets vector

			for _, blockDesc := range eccDesc.Blocks {
				// Emit a BlockSetDesc
				str += fmt.Sprintf("{%d,%d,%d},",
					blockDesc.Num, blockDesc.CodeNumTotal,
					blockDesc.CodeNumData)
			}
			str += "}"  // end the block_sets vector
			str += "}," // end the LevelDesc

		}
		str += "} },\n"

		if err := writeString(w, str); err != nil {
			return err
		}
	}

	post := strings.ReplaceAll(headerPostamble, "GUARD", *headerGuardFlag)
	if err := writeString(w, post); err != nil {
		return err
	}

	return nil
}

func main() {
	flag.Parse()

	if *outputFlag == "" {
		log.Fatal("--output is required")
	}
	if *headerGuardFlag == "" {
		log.Fatal("--header_guard is required")
	}

	lines, err := readInput(*inputFlag)
	if err != nil {
		log.Fatal(err)
	}

	descs, err := parseInput(lines)
	if err != nil {
		log.Fatal(err)
	}

	if err := validateDescs(descs); err != nil {
		log.Fatal(err)
	}

	if *outputFlag == "-" {
		writeHeader(descs, os.Stdout)
	} else {
		fp, err := os.Create(*outputFlag)
		if err != nil {
			log.Fatal(err)
		}

		w := bufio.NewWriter(fp)
		if err := writeHeader(descs, w); err != nil {
			fp.Close()
			log.Fatalf("failed to write header: %v", err)
		}
		w.Flush()

		if err := fp.Close(); err != nil {
			log.Fatalf("failed to close header: %v", err)
		}
	}
}
