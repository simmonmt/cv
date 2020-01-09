#include "qrcode/gf.h"

#include "absl/base/macros.h"
#include "gtest/gtest.h"

#include "qrcode/bits.h"

namespace {

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

TEST(GF16Test, Add) {
  GF16 gf16;

  EXPECT_EQ(b0000, gf16.Add({b0000, b0000}));
  EXPECT_EQ(b1111, gf16.Add({b0101, b1010}));
  EXPECT_EQ(b0101, gf16.Add({b1111, b1010}));
  EXPECT_EQ(b1010, gf16.Add({b1111, b0101}));
  EXPECT_EQ(b0000, gf16.Add({b1111, b1111}));
}

TEST(GF16Test, Mult) { TestMult(GF16()); }

TEST(GF16Test, Exp) {
  GF16 gf16;

  EXPECT_EQ(b0000, gf16.Exp(b0000, 2));
  EXPECT_EQ(gf16.PowersOfAlpha()[4], gf16.Exp(gf16.PowersOfAlpha()[2], 2));

  // alpha^10^2 = alpha^20 = alpha^5
  EXPECT_EQ(gf16.PowersOfAlpha()[5], gf16.Exp(gf16.PowersOfAlpha()[10], 2));
}

TEST(GF256Test, Add) {
  GF256 gf;

  EXPECT_EQ(0b00000000, gf.Add({0b00000000, 0b00000000}));
  EXPECT_EQ(0b11111111, gf.Add({0b01010101, 0b10101010}));
  EXPECT_EQ(0b01010101, gf.Add({0b11111111, 0b10101010}));
  EXPECT_EQ(0b10101010, gf.Add({0b11111111, 0b01010101}));
  EXPECT_EQ(0b00000000, gf.Add({0b11111111, 0b11111111}));
}

TEST(GF256Test, Mult) { TestMult(GF256()); }

TEST(GF256Test, Exp) {
  GF256 gf;

  EXPECT_EQ(0b00000000, gf.Exp(0b00000000, 2));
  EXPECT_EQ(gf.PowersOfAlpha()[4], gf.Exp(gf.PowersOfAlpha()[2], 2));

  for (int i = 0; i < 256; ++i) {
    if (gf.PowersOfAlpha()[i] == gf.Exp(gf.PowersOfAlpha()[200], 2)) {
      std::cout << "found " << i << "\n";
    }
  }

  // alpha^200^2 = alpha^400 = alpha^145
  EXPECT_EQ(gf.PowersOfAlpha()[145], gf.Exp(gf.PowersOfAlpha()[200], 2));
}

}  // namespace
