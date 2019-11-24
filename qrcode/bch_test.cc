#include "qrcode/bch.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "qrcode/gf.h"

namespace {

using ::testing::ElementsAreArray;
using ::testing::VariantWith;

TEST(BCHDecoderTest, Test15_5) {
  const std::vector<bool> ref = {0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1};

  // No errors
  EXPECT_THAT(DecodeBCH(GF16(), ref),
              VariantWith<std::vector<bool>>(ElementsAreArray(ref)));

  // Two errors
  std::vector<bool> input = ref;
  input[5] = !input[5];
  input[13] = !input[13];
  EXPECT_THAT(DecodeBCH(GF16(), input),
              VariantWith<std::vector<bool>>(ElementsAreArray(ref)));
}

TEST(BCHDecoderTest, Test18_6) {
  const std::vector<bool> ref = {0, 0, 1, 0, 1, 0, 0, 1, 0,
                                 0, 1, 1, 1, 1, 1, 0, 0, 0};

  EXPECT_THAT(DecodeBCH(GF32(), ref),
              VariantWith<std::vector<bool>>(ElementsAreArray(ref)));

  // Two errors
  std::vector<bool> input = ref;
  input[5] = !input[5];
  input[13] = !input[13];
  EXPECT_THAT(DecodeBCH(GF32(), input),
              VariantWith<std::vector<bool>>(ElementsAreArray(ref)));
}

}  // namespace
