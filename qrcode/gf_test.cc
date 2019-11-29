#include "qrcode/gf.h"

#include "absl/base/macros.h"
#include "gtest/gtest.h"

#include "qrcode/bits.h"

namespace {

TEST(GF16Test, Add) {
  GF16 gf16;

  EXPECT_EQ(b0000, gf16.Add({b0000, b0000}));
  EXPECT_EQ(b1111, gf16.Add({b0101, b1010}));
  EXPECT_EQ(b0101, gf16.Add({b1111, b1010}));
  EXPECT_EQ(b1010, gf16.Add({b1111, b0101}));
  EXPECT_EQ(b0000, gf16.Add({b1111, b1111}));
}

void TestMult(const GF& gf) {
  const std::vector<unsigned char>& powers_of_alpha = gf.PowersOfAlpha();

  for (int i = 0; i < powers_of_alpha.size(); ++i) {
    for (int j = 0; j < powers_of_alpha.size(); ++j) {
      const unsigned char m1 = powers_of_alpha[i];
      const unsigned char m2 = powers_of_alpha[j];
      const unsigned char res = gf.Power(i + j);

      EXPECT_EQ(res, gf.Mult(m1, m2))
          << "i=" << i << ",j=" << j << " " << int(m1) << "*" << int(m2) << "="
          << int(res);

      if (i == j) {
        EXPECT_EQ(res, gf.Exp(m1, 2)) << "square " << i << " " << int(m1);
      }
    }
  }

  for (int i = 0; i < 100; ++i) {
    // Verify that alpha^n * alpha == alpha^n+1
    EXPECT_EQ(gf.Power(i + 1), gf.Mult(gf.Power(i), gf.Power(1)));
  }
}

TEST(GF16Test, Mult) { TestMult(GF16()); }

TEST(GF16Test, Exp) {
  GF16 gf16;

  EXPECT_EQ(b0000, gf16.Exp(b0000, 2));
  EXPECT_EQ(gf16.PowersOfAlpha()[4], gf16.Exp(gf16.PowersOfAlpha()[2], 2));

  // alpha^10^2 = alpha^20 = alpha^5
  EXPECT_EQ(gf16.PowersOfAlpha()[5], gf16.Exp(gf16.PowersOfAlpha()[10], 2));
}

TEST(GF32Test, Add) {
  GF32 gf32;

  EXPECT_EQ(b00000, gf32.Add({b00000, b00000}));
  EXPECT_EQ(b11111, gf32.Add({b01010, b10101}));
  EXPECT_EQ(b01010, gf32.Add({b11111, b10101}));
  EXPECT_EQ(b10101, gf32.Add({b11111, b01010}));
  EXPECT_EQ(b00000, gf32.Add({b11111, b11111}));
}

TEST(GF32Test, Mult) { TestMult(GF32()); }

TEST(GF32Test, Exp) {
  GF32 gf32;

  EXPECT_EQ(b00000, gf32.Exp(b00000, 2));
  EXPECT_EQ(gf32.PowersOfAlpha()[4], gf32.Exp(gf32.PowersOfAlpha()[2], 2));

  // alpha^10^5 = alpha^50 = alpha^9
  EXPECT_EQ(gf32.PowersOfAlpha()[19], gf32.Exp(gf32.PowersOfAlpha()[10], 5));
}

}  // namespace
