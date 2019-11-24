#include "qrcode/qr_decode_utils.h"

#include <vector>

#include "absl/base/macros.h"
#include "absl/strings/str_format.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "qrcode/bits.h"
#include "qrcode/gf.h"

namespace {

using ::testing::ElementsAre;

class Calc {
 public:
  static unsigned char R(const std::vector<bool> poly, int alpha_power) {
    unsigned char out_bits = 0;
    for (int i = poly.size() - 1; i >= 0; --i) {
      if (poly[i]) {
        const int i_power = ((14 - i) * alpha_power) % 15;
        const unsigned char to_add = GF16().PowersOfAlpha()[i_power];
        out_bits = GF16().Add({out_bits, to_add});
      }
    }
    return out_bits;
  }
};

class CalcTest : public ::testing::Test {
 public:
  CalcTest()
      // These examples are from
      // https://en.wikipedia.org/wiki/BCH_code#Decoding_of_binary_code_without_unreadable_characters
      // Note: these vectors are MSB-first.
      : input_no_errors_({1, 1, 0, 1, 1, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0}),
        // Has error at bit index 7
        input_one_error_({1, 1, 0, 1, 1, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0}),
        // Has error at bit indexes 5 and 13.
        input_two_errors_({1, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 1, 0, 0}) {}

  std::vector<bool> input_no_errors_, input_one_error_, input_two_errors_;
};

TEST_F(CalcTest, R) {
  {
    // No errors, same source.
    EXPECT_EQ(b0000, Calc::R(input_no_errors_, 1));
    EXPECT_EQ(b0000, Calc::R(input_no_errors_, 3));
    EXPECT_EQ(b0000, Calc::R(input_no_errors_, 5));
  }

  {
    EXPECT_EQ(b1011, Calc::R(input_two_errors_, 1));
    EXPECT_EQ(b1011, Calc::R(input_two_errors_, 3));
    EXPECT_EQ(b0001, Calc::R(input_two_errors_, 5));
  }
}

TEST_F(CalcTest, Decode) {
  GF16 gf16;

  const unsigned char s1 = Calc::R(input_two_errors_, 1);
  const unsigned char s3 = Calc::R(input_two_errors_, 3);
  const unsigned char s5 = Calc::R(input_two_errors_, 5);
  ASSERT_EQ(b1011, s1);
  ASSERT_EQ(b1011, s3);
  ASSERT_EQ(b0001, s5);

  const unsigned char s2 = gf16.Pow(s1, 2);
  const unsigned char s4 = gf16.Pow(s2, 2);

  // Solve Eq1: S1 + d1 = 0
  const unsigned char d1 = s1;

  // Verify Eq1
  ASSERT_EQ(0, gf16.Add({s1, d1}));

  // Eq2: S3 + S2*d1 + S1*d2 + d3 = 0
  const unsigned char s3_plus_s2d1 = gf16.Add({s3, gf16.Mult(s2, d1)});
  auto eq2 = [&](unsigned char d2, unsigned char d3) {
    unsigned char res = s3_plus_s2d1;
    res = gf16.Add({res, gf16.Mult(s1, d2)});
    res = gf16.Add({res, d3});
    return res;
  };

  // Eq3: S5 + S4*d1 + S3*d2 + S2*d3 = 0
  const unsigned char s5_plus_s4d1 = gf16.Add({s5, gf16.Mult(s4, d1)});
  auto eq3 = [&](unsigned char d2, unsigned char d3) {
    unsigned char res = s5_plus_s4d1;
    res = gf16.Add({res, gf16.Mult(s3, d2)});
    res = gf16.Add({res, gf16.Mult(s2, d3)});
    return res;
  };

  bool found = false;
  unsigned char d2, d3;
  const std::vector<unsigned char> gf_elements = gf16.Elements();
  for (int i = 0; !found && i < gf_elements.size(); ++i) {
    for (int j = 0; !found && j < gf_elements.size(); ++j) {
      d2 = gf_elements[i];
      d3 = gf_elements[j];
      if (eq2(d2, d3) == 0 && eq3(d2, d3) == 0) {
        found = true;
      }
    }
  }
  ASSERT_TRUE(found);

  // Verify eq2 and eq3
  ASSERT_EQ(0, gf16.Add({s3,                 //
                         gf16.Mult(s2, d1),  //
                         gf16.Mult(s1, d2),  //
                         d3}));
  ASSERT_EQ(0, gf16.Add({s5,                 //
                         gf16.Mult(s4, d1),  //
                         gf16.Mult(s3, d2),  //
                         gf16.Mult(s2, d3)}));

  std::vector<int> errors;
  const std::vector<unsigned char>& powers_of_alpha = gf16.PowersOfAlpha();
  for (int i = 0; i < powers_of_alpha.size(); ++i) {
    // Look for x^3 + d1*x^2 + d2*x + d3 == 0
    unsigned char x = powers_of_alpha[i];
    unsigned char res = gf16.Add({gf16.Pow(x, 3),                 //
                                  gf16.Mult(d1, gf16.Pow(x, 2)),  //
                                  gf16.Mult(d2, x),               //
                                  d3});

    if (res == 0) {
      errors.push_back(i);
    }
  }

  EXPECT_THAT(errors, ElementsAre(5, 13));
}

}  // namespace
