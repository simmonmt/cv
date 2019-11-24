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

TEST(GF16Test, Mult) {
  GF16 gf16;

  const std::vector<unsigned char>& powers_of_alpha = gf16.PowersOfAlpha();
  for (int i = 0; i < powers_of_alpha.size(); ++i) {
    for (int j = 0; j < powers_of_alpha.size(); ++j) {
      const unsigned char m1 = powers_of_alpha[i];
      const unsigned char m2 = powers_of_alpha[j];
      const unsigned char res = powers_of_alpha[(i + j) % 15];

      EXPECT_EQ(res, gf16.Mult(m1, m2))
          << "i=" << i << ",j=" << j << " " << int(m1) << "*" << int(m2) << "="
          << int(res);

      if (i == j) {
        EXPECT_EQ(res, gf16.Pow(m1, 2)) << "square " << i << " " << int(m1);
      }
    }
  }
}

TEST(GF16Test, Pow) {
  GF16 gf16;

  EXPECT_EQ(b0000, gf16.Pow(b0000, 2));
  EXPECT_EQ(gf16.PowersOfAlpha()[4], gf16.Pow(gf16.PowersOfAlpha()[2], 2));

  // alpha^10^2 = alpha^20 = alpha^5
  EXPECT_EQ(gf16.PowersOfAlpha()[5], gf16.Pow(gf16.PowersOfAlpha()[10], 2));
}

TEST(GF16Test, Enumerate) {
  GF16 gf16;
  unsigned char cur = 1, mult = 2;

  for (int i = 0; i < 20; ++i) {
    std::cout << "i=" << i << " cur=" << int(cur) << "\n";
    cur = gf16.Mult(cur, mult);
  }
}

}  // namespace
