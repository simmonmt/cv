#include "qrcode/qr_decode_utils.h"

#include <iostream>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "qrcode/testutils.h"

namespace {

using ::testing::ElementsAreArray;

constexpr char kTestDataSpecExamplePath[] =
    "qrcode/testdata/spec_example_1m.txt";

class DecodeFormatTest : public ::testing::Test {
 public:
  void SetUp() override {
    ASSIGN_OR_ASSERT(base_, ReadQRCodeArrayFromFile(kTestDataSpecExamplePath),
                     "read failed");
  }

  std::unique_ptr<QRCodeArray> base_;
};

TEST_F(DecodeFormatTest, Test) {
  ASSIGN_OR_ASSERT(QRFormat format, DecodeFormat(*base_), "decode failed");

  // The spec says it's mask pattern 010, but it's actually 011 because we can't
  // have nice things.
  EXPECT_EQ(QRECC_M, format.ecc_level);
  EXPECT_EQ(0b011, format.mask_pattern);
}

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

}  // namespace
