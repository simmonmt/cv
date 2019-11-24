#include "qrcode/gf.h"

#include "absl/base/macros.h"
#include "gtest/gtest.h"

#include "qrcode/bits.h"

namespace {

TEST(GF16Test, Add) {
  EXPECT_EQ(b0000, GF16::Add({b0000, b0000}));
  EXPECT_EQ(b1111, GF16::Add({b0101, b1010}));
  EXPECT_EQ(b0101, GF16::Add({b1111, b1010}));
  EXPECT_EQ(b1010, GF16::Add({b1111, b0101}));
  EXPECT_EQ(b0000, GF16::Add({b1111, b1111}));
}

TEST(GF16Test, Mult) {
  for (int i = 0; i < ABSL_ARRAYSIZE(GF16::kPowersOfAlpha); ++i) {
    for (int j = 0; j < ABSL_ARRAYSIZE(GF16::kPowersOfAlpha); ++j) {
      const unsigned char m1 = GF16::kPowersOfAlpha[i];
      const unsigned char m2 = GF16::kPowersOfAlpha[j];
      const unsigned char res = GF16::kPowersOfAlpha[(i + j) % 15];

      EXPECT_EQ(res, GF16::Mult(m1, m2))
          << "i=" << i << ",j=" << j << " " << int(m1) << "*" << int(m2) << "="
          << int(res);

      if (i == j) {
        EXPECT_EQ(res, GF16::Pow(m1, 2)) << "square " << i << " " << int(m1);
      }
    }
  }
}

TEST(GF16Test, Pow) {
  EXPECT_EQ(b0000, GF16::Pow(b0000, 2));
  EXPECT_EQ(GF16::kPowersOfAlpha[4], GF16::Pow(GF16::kPowersOfAlpha[2], 2));

  // alpha^10^2 = alpha^20 = alpha^5
  EXPECT_EQ(GF16::kPowersOfAlpha[5], GF16::Pow(GF16::kPowersOfAlpha[10], 2));
}

}  // namespace
