#include "qrcode/qr_decode_utils.h"

#include <iostream>

#include "absl/strings/str_cat.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "qrcode/testutils.h"

namespace {

using ::testing::ElementsAre;
using ::testing::ElementsAreArray;
using ::testing::Field;
using ::testing::SizeIs;
using ::testing::VariantWith;

constexpr char kTestDataSpecExampleUnmaskedPath[] =
    "qrcode/testdata/spec_example_1m_unmasked.txt";
constexpr char kTestDataV2H[] = "qrcode/testdata/v2h.txt";

class UnmaskingTest : public ::testing::Test {
 public:
  void VerifyUnmasking(int version, std::unique_ptr<QRCodeArray> array,
                       unsigned char mask_pattern,
                       const std::vector<std::string>& expected_strs) {
    auto attributes =
        absl::get<std::unique_ptr<QRAttributes>>(QRAttributes::New(1, QRECC_L));

    UnmaskArray(*attributes, array.get(), mask_pattern);

    const std::vector<std::string> out_strs = QRCodeArrayToStrings(*array);

    std::cout << "version " << version << " mask " << int(mask_pattern)
              << " result:\n";
    for (int i = 0; i < out_strs.size(); i++) {
      std::cout << i << ":\t" << out_strs[i] << "\n";
    }

    EXPECT_THAT(out_strs, ElementsAreArray(expected_strs))
        << "version " << version << " mask " << int(mask_pattern);
  }

  void VerifyUnmasking(int version, const std::vector<std::string>& in_strs,
                       unsigned char mask_pattern,
                       const std::vector<std::string>& expected_strs) {
    std::unique_ptr<QRCodeArray> in = ReadQRCodeArrayFromStrings(in_strs);

    VerifyUnmasking(version, std::move(in), mask_pattern, expected_strs);
  }
};

// TODO: Add tests for missing mask patterns.
// We have 000, 011, 111. Need 001, 010, 100, 101, 110.

TEST_F(UnmaskingTest, Test) {
  struct TestCase {
    std::vector<std::string> masked;
    int mask_pattern;
  };

  const TestCase kTestCases[] = {
      {
          {
              "XXXXXXX  X X  XXXXXXX",  //
              "X     X   X X X     X",  //
              "X XXX X  X X  X XXX X",  //
              "X XXX X   X X X XXX X",  //
              "X XXX X  X X  X XXX X",  //
              "X     X  X X  X     X",  //
              "XXXXXXX X X X XXXXXXX",  //
              "         X X         ",  //
              "      X   X X        ",  //
              "X X X   X X X X X X X",  //
              " X X XXX X X X X X X ",  //
              "X X X   X X X X X X X",  //
              "X X X X X X X X X X X",  //
              "         X X X X X X ",  //
              "XXXXXXX  XXX XXXX X X",  //
              "X     X     X    X X ",  //
              "X XXX X  XXX XXXX X X",  //
              "X XXX X     X    X X ",  //
              "X XXX X  XXX XXXX X X",  //
              "X     X     X    X X ",  //
              "XXXXXXX  XXX XXXX X X",  //
          },
          0b000,
      },
      {
          {
              "XXXXXXX   XX  XXXXXXX",  //
              "X     X  XX X X     X",  //
              "X XXX X  X XX X XXX X",  //
              "X XXX X   XX  X XXX X",  //
              "X XXX X  XX X X XXX X",  //
              "X     X   X   X     X",  //
              "XXXXXXX X X X XXXXXXX",  //
              "           X         ",  //
              "      X   X          ",  //
              " XX XX XX XX XX XX XX",  //
              "XX XX XX XX XX XX XX ",  //
              "X XX X  XX XX XX XX X",  //
              "X  X  X  X  X  X  X  ",  //
              "           X  X  X  X",  //
              "XXXXXXX  XXXX  XX  X ",  //
              "X     X    X X    X  ",  //
              "X XXX X  X  XXXX X  X",  //
              "X XXX X  XXXX  XX  X ",  //
              "X XXX X    X X    X  ",  //
              "X     X  X  XXXX X  X",  //
              "XXXXXXX  XXXX  XX  X ",  //
          },
          0b011,
      },
      {
          {
              "XXXXXXX  X X  XXXXXXX",  //
              "X     X     X X     X",  //
              "X XXX X  X    X XXX X",  //
              "X XXX X   X X X XXX X",  //
              "X XXX X  XXX  X XXX X",  //
              "X     X  X    X     X",  //
              "XXXXXXX X X X XXXXXXX",  //
              "         XXX         ",  //
              "      X   XXX        ",  //
              "X X X   X X X X X X X",  //
              "   XXXX  XXX   XXX   ",  //
              "X   XX    XXX   XXX  ",  //
              "X X X X X X X X X X X",  //
              "         XXX   XXX   ",  //
              "XXXXXXX  XX  X XXXX  ",  //
              "X     X     X    X X ",  //
              "X XXX X  X X  XX  XXX",  //
              "X XXX X    XX X    XX",  //
              "X XXX X  XXX XXXX X X",  //
              "X     X   X XX  XX   ",  //
              "XXXXXXX  XX  X XXXX  ",  //
          },
          0b111,
      },
  };

  const std::vector<std::string> expected = {
      "XXXXXXX  XXXX XXXXXXX",  //
      "X     X  XXXX X     X",  //
      "X XXX X  XXXX X XXX X",  //
      "X XXX X  XXXX X XXX X",  //
      "X XXX X  XXXX X XXX X",  //
      "X     X       X     X",  //
      "XXXXXXX X X X XXXXXXX",  //
      "                     ",  //
      "      X              ",  //
      "XXXXXX XXXXXXXXXXXXXX",  //
      "XXXXXXXXXXXXXXXXXXXXX",  //
      "XXXXXX XXXXXXXXXXXXXX",  //
      "      X              ",  //
      "                     ",  //
      "XXXXXXX  X XXX X     ",  //
      "X     X  X XXX X     ",  //
      "X XXX X  X XXX X     ",  //
      "X XXX X  X XXX X     ",  //
      "X XXX X  X XXX X     ",  //
      "X     X  X XXX X     ",  //
      "XXXXXXX  X XXX X     ",  //
  };

  for (const TestCase& test_case : kTestCases) {
    VerifyUnmasking(1, test_case.masked, test_case.mask_pattern, expected);
  }
}

class FindCodewordsTest : public ::testing::Test {
 public:
  absl::variant<std::vector<unsigned char>, std::string> CallFindCodewords(
      const QRCodeArray& array, int version) {
    auto result = QRAttributes::New(version, QRECC_L);
    if (absl::holds_alternative<std::string>(result)) {
      const std::string msg = absl::get<std::string>(result);
      std::cout << "QRAttributes init failed: " << msg;
      return msg;
    }
    auto attributes =
        std::move(absl::get<std::unique_ptr<QRAttributes>>(result));

    return FindCodewords(*attributes, array);
  }
};

TEST_F(FindCodewordsTest, V1) {
  ASSIGN_OR_ASSERT(std::unique_ptr<QRCodeArray> array,
                   ReadQRCodeArrayFromFile(kTestDataSpecExampleUnmaskedPath),
                   "read failed");

  const std::vector<unsigned char> expected = {
      0x10, 0x20, 0x0c, 0x56, 0x61, 0x80, 0xec, 0x11, 0xec, 0x11,  //
      0xec, 0x11, 0xec, 0x11, 0xec, 0x11, 0xa5, 0x24, 0xd4, 0xc1,  //
      0xed, 0x36, 0xc7, 0x87, 0x2c, 0x55};

  EXPECT_THAT(
      CallFindCodewords(*array, 1),
      VariantWith<std::vector<unsigned char>>(ElementsAreArray(expected)));
}

TEST_F(FindCodewordsTest, V2) {
  ASSIGN_OR_ASSERT(std::unique_ptr<QRCodeArray> array,
                   ReadQRCodeArrayFromFile(kTestDataV2H), "read failed");

  // V2 has remainder bits. We want to make sure they're not returned. If they
  // are, we'll get 45.
  EXPECT_THAT(CallFindCodewords(*array, 2),
              VariantWith<std::vector<unsigned char>>(SizeIs(44)));
}

class SplitCodewordsTest : public ::testing::Test {
 public:
  // Returns a sequence a .. b (inclusive).
  std::vector<unsigned char> MakeSequence(int a, int b) {
    std::vector<unsigned char> out;
    for (int i = a; i <= b; i++) {
      out.push_back(i);
    }
    return out;
  }
};

TEST_F(SplitCodewordsTest, V5H) {
  ASSIGN_OR_ASSERT(std::unique_ptr<QRAttributes> attributes,
                   QRAttributes::New(5, QRECC_H), "attr fail");

  // ECC codewords follow the data codewords, of which there are 46.
  auto ecc = [](int num) -> unsigned char { return num + 46; };

  const std::vector<unsigned char> unordered = {
      1,       12,      23,      35,  //
      2,       13,      24,      36,  //
      3,       14,      25,      37,  //
      4,       15,      26,      38,  //
      5,       16,      27,      39,  //
      6,       17,      28,      40,  //
      7,       18,      29,      41,  //
      8,       19,      30,      42,  //
      9,       20,      31,      43,  //
      10,      21,      32,      44,  //
      11,      22,      33,      45,  //
      34,      46,                    //

      ecc(1),  ecc(23), ecc(45), ecc(67),  //
      ecc(2),  ecc(24), ecc(46), ecc(68),  //
      ecc(3),  ecc(25), ecc(47), ecc(69),  //
      ecc(4),  ecc(26), ecc(48), ecc(70),  //
      ecc(5),  ecc(27), ecc(49), ecc(71),  //
      ecc(6),  ecc(28), ecc(50), ecc(72),  //
      ecc(7),  ecc(29), ecc(51), ecc(73),  //
      ecc(8),  ecc(30), ecc(52), ecc(74),  //
      ecc(9),  ecc(31), ecc(53), ecc(75),  //
      ecc(10), ecc(32), ecc(54), ecc(76),  //
      ecc(11), ecc(33), ecc(55), ecc(77),  //
      ecc(12), ecc(34), ecc(56), ecc(78),  //
      ecc(13), ecc(35), ecc(57), ecc(79),  //
      ecc(14), ecc(36), ecc(58), ecc(80),  //
      ecc(15), ecc(37), ecc(59), ecc(81),  //
      ecc(16), ecc(38), ecc(60), ecc(82),  //
      ecc(17), ecc(39), ecc(61), ecc(83),  //
      ecc(18), ecc(40), ecc(62), ecc(84),  //
      ecc(19), ecc(41), ecc(63), ecc(85),  //
      ecc(20), ecc(42), ecc(64), ecc(86),  //
      ecc(21), ecc(43), ecc(65), ecc(87),  //
      ecc(22), ecc(44), ecc(66), ecc(88),  //
  };

  EXPECT_THAT(
      SplitCodewordsIntoBlocks(attributes->error_characteristics(), unordered),
      ElementsAre(
          AllOf(Field(&CodewordBlock::data,
                      ElementsAreArray(MakeSequence(1, 11))),
                Field(&CodewordBlock::ecc,
                      ElementsAreArray(MakeSequence(ecc(1), ecc(22))))),
          AllOf(Field(&CodewordBlock::data,
                      ElementsAreArray(MakeSequence(12, 22))),
                Field(&CodewordBlock::ecc,
                      ElementsAreArray(MakeSequence(ecc(23), ecc(44))))),
          AllOf(Field(&CodewordBlock::data,
                      ElementsAreArray(MakeSequence(23, 34))),
                Field(&CodewordBlock::ecc,
                      ElementsAreArray(MakeSequence(ecc(45), ecc(66))))),
          AllOf(Field(&CodewordBlock::data,
                      ElementsAreArray(MakeSequence(35, 46))),
                Field(&CodewordBlock::ecc,
                      ElementsAreArray(MakeSequence(ecc(67), ecc(88)))))));
}

}  // namespace
