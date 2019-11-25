#include "qrcode/bch.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "qrcode/gf.h"

namespace {

using ::testing::ElementsAreArray;
using ::testing::VariantWith;

// For whatever reason VariantWith doesn't print the string (i.e. the other
// value), so this wrapper does it.
absl::variant<std::vector<bool>, std::string> CallDecodeBCH(
    const GF& gf, const std::vector<bool>& bits) {
  auto out = DecodeBCH(gf, bits);
  if (absl::holds_alternative<std::string>(out)) {
    std::cout << "decoder returned error: " << absl::get<std::string>(out)
              << "\n";
  }
  return out;
}

TEST(BCHDecoderTest, Test15_5_Wikipedia) {
  // Example from
  // https://en.wikipedia.org/wiki/BCH_code#Decoding_of_binary_code_without_unreadable_characters
  // In reverse order because we want LSB first.
  const std::vector<bool> ref = {0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1};

  // No errors
  EXPECT_THAT(CallDecodeBCH(GF16(), ref),
              VariantWith<std::vector<bool>>(ElementsAreArray(ref)));

  // Two errors
  std::vector<bool> input = ref;
  input[5] = !input[5];
  input[13] = !input[13];
  EXPECT_THAT(CallDecodeBCH(GF16(), input),
              VariantWith<std::vector<bool>>(ElementsAreArray(ref)));
}

TEST(BCHDecoderTest, Test15_5_QRStandard) {
  // Error Correction level M, Mask Pattern 101
  // Example from Annex C in the 18004:2000(E) standard
  const std::vector<bool> ref = {0, 0, 1, 1, 1, 0, 1, 1, 0, 0, 1, 0, 1, 0, 0};
  EXPECT_THAT(CallDecodeBCH(GF16(), ref),
              VariantWith<std::vector<bool>>(ElementsAreArray(ref)));
}

TEST(BCHDecoderTest, Test18_6) {
  // const std::vector<bool> ref = {0, 0, 1, 0, 1, 0, 0, 1, 0,
  //                                0, 1, 1, 1, 1, 1, 0, 0, 0};
  const std::vector<bool> ref = {0, 0, 0, 1, 1, 1, 1, 1, 0,
                                 0, 1, 0, 0, 1, 0, 1, 0, 0};

  EXPECT_THAT(CallDecodeBCH(GF32(), ref),
              VariantWith<std::vector<bool>>(ElementsAreArray(ref)));
  return;

  // Two errors
  std::vector<bool> input = ref;
  input[5] = !input[5];
  input[13] = !input[13];
  EXPECT_THAT(CallDecodeBCH(GF32(), input),
              VariantWith<std::vector<bool>>(ElementsAreArray(ref)));
}

}  // namespace
