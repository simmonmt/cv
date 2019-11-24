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
        const unsigned char to_add = GF16::kPowersOfAlpha[i_power];
        out_bits = GF16::Add({out_bits, to_add});
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
  const unsigned char s1 = Calc::R(input_two_errors_, 1);
  const unsigned char s3 = Calc::R(input_two_errors_, 3);
  const unsigned char s5 = Calc::R(input_two_errors_, 5);
  ASSERT_EQ(b1011, s1);
  ASSERT_EQ(b1011, s3);
  ASSERT_EQ(b0001, s5);

  const unsigned char s2 = GF16::Pow(s1, 2);
  const unsigned char s4 = GF16::Pow(s2, 2);

  // Solve Eq1: S1 + d1 = 0
  const unsigned char d1 = s1;  // (~s1) & 0xf;

  // Verify Eq1
  ASSERT_EQ(0, GF16::Add({s1, d1}));

  // Eq2: S3 + S2*d1 + S1*d2 + d3 = 0
  const unsigned char s3_plus_s2d1 = GF16::Add({s3, GF16::Mult(s2, d1)});
  auto eq2 = [&](unsigned char d2, unsigned char d3) {
    unsigned char res = s3_plus_s2d1;
    res = GF16::Add({res, GF16::Mult(s1, d2)});
    res = GF16::Add({res, d3});
    return res;
  };

  // Eq3: S5 + S4*d1 + S3*d2 + S2*d3 = 0
  const unsigned char s5_plus_s4d1 = GF16::Add({s5, GF16::Mult(s4, d1)});
  auto eq3 = [&](unsigned char d2, unsigned char d3) {
    unsigned char res = s5_plus_s4d1;
    res = GF16::Add({res, GF16::Mult(s3, d2)});
    res = GF16::Add({res, GF16::Mult(s2, d3)});
    return res;
  };

  bool found = false;
  unsigned char d2, d3;
  for (int i = 0; !found && i < ABSL_ARRAYSIZE(GF16::kElements); ++i) {
    for (int j = 0; !found && j < ABSL_ARRAYSIZE(GF16::kElements); ++j) {
      d2 = GF16::kElements[i];
      d3 = GF16::kElements[j];
      if (eq2(d2, d3) == 0 && eq3(d2, d3) == 0) {
        found = true;
      }
    }
  }
  ASSERT_TRUE(found);

  // Verify eq2 and eq3
  ASSERT_EQ(0, GF16::Add({s3,                  //
                          GF16::Mult(s2, d1),  //
                          GF16::Mult(s1, d2),  //
                          d3}));
  ASSERT_EQ(0, GF16::Add({s5,                  //
                          GF16::Mult(s4, d1),  //
                          GF16::Mult(s3, d2),  //
                          GF16::Mult(s2, d3)}));

  std::vector<int> errors;
  for (int i = 0; i < ABSL_ARRAYSIZE(GF16::kPowersOfAlpha); ++i) {
    // Look for x^3 + d1*x^2 + d2*x + d3 == 0
    unsigned char x = GF16::kPowersOfAlpha[i];
    unsigned char res = GF16::Add({GF16::Pow(x, 3),                  //
                                   GF16::Mult(d1, GF16::Pow(x, 2)),  //
                                   GF16::Mult(d2, x),                //
                                   d3});

    if (res == 0) {
      errors.push_back(i);
    }
  }

  EXPECT_THAT(errors, ElementsAre(5, 13));
}

}  // namespace
