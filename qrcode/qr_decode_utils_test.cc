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

TEST_F(UnmaskingTest, Test1) {
  const std::vector<std::string> masked_000 = {
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
  };

  const std::vector<std::string> masked_111 = {
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

  VerifyUnmasking(1, masked_000, 0, expected);
  VerifyUnmasking(1, masked_111, 7, expected);
}

}  // namespace
