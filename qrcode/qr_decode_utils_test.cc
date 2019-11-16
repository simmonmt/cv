#include "qrcode/qr_decode_utils.h"

#include <vector>

#include "absl/strings/str_format.h"
#include "gtest/gtest.h"

namespace {

enum {
  b0000,
  b0001,
  b0010,
  b0011,
  b0100,
  b0101,
  b0110,
  b0111,
  b1000,
  b1001,
  b1010,
  b1011,
  b1100,
  b1101,
  b1110,
  b1111,
};

class Calc {
 public:
  // This array contains elements of GF(2^4). Element i is alpha^i,
  // which takes the form
  //
  //    a + b*alpha + c*alpha^2 + d*alpha^3
  //
  // for values of a, b, c, d in GF(2) (or 0, 1). The elements are
  // stored bitwise in the array as d, c, b, a.
  //
  // Examples:
  //   i=0 is alpha^0 => 1,       stored as 0b0001
  //   i=1 is alpha^1 => alpha,   stored as 0b0010
  //   ...
  //   i=3 is alpha^3 => alpha^3, stored as 0b1000
  //   i=4 is alpha^4 => alpha+1, stored as 0b0011
  //   ...
  static constexpr unsigned char kGFBits[15] = {
      1, 2, 4, 8, 3, 6, 12, 11, 5, 10, 7, 14, 15, 13, 9,
  };

  // Reverses the mapping in kGFBits.
  //    kGFIndexes[kGFBits[i]] = i
  // 0th value is sentinel since no kGFBits element = 0.
  static constexpr unsigned char kGFIndexes[16] = {
      255, 0, 1, 4, 2, 8, 5, 10, 3, 14, 9, 7, 6, 13, 11, 12,
  };

  // input has 15 bits, MSB (x^14) first.
  Calc(const std::vector<bool>& input) : input_(input){};

  static unsigned char add(unsigned char a, unsigned char b) { return a ^ b; }

  static unsigned char square(unsigned char a) {
    return kGFBits[(kGFIndexes[a] * 2) % 15];
  }

  unsigned char r(int alpha_power) const {
    unsigned char out_bits = 0;
    for (int i = input_.size() - 1; i >= 0; --i) {
      if (input_[i]) {
        const int i_power = ((14 - i) * alpha_power) % 15;
        const unsigned char to_add = kGFBits[i_power];
        out_bits = add(out_bits, to_add);
      }
    }
    return out_bits;
  }

  const std::vector<bool> input_;
};

constexpr unsigned char Calc::kGFBits[15];
constexpr unsigned char Calc::kGFIndexes[16];

class CalcTest : public ::testing::Test {
 public:
  CalcTest()
      : calc_errors_({1, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 1, 0, 0}),
        calc_no_errors_({1, 1, 0, 1, 1, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0}) {}

  Calc calc_errors_, calc_no_errors_;
};

TEST_F(CalcTest, R) {
  {
    // Example with two errors from
    //
    // https://en.wikipedia.org/wiki/BCH_code#Decoding_of_binary_code_without_unreadable_characters
    EXPECT_EQ(b1011, calc_errors_.r(1));
    EXPECT_EQ(b1011, calc_errors_.r(3));
    EXPECT_EQ(b0001, calc_errors_.r(5));
  }

  {
    // No errors, same source.
    Calc calc({1, 1, 0, 1, 1, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0});
    EXPECT_EQ(b0000, calc_no_errors_.r(1));
    EXPECT_EQ(b0000, calc_no_errors_.r(3));
    EXPECT_EQ(b0000, calc_no_errors_.r(5));
  }
}

TEST_F(CalcTest, add) {
  EXPECT_EQ(b0000, Calc::add(b0000, b0000));
  EXPECT_EQ(b1111, Calc::add(b0101, b1010));
  EXPECT_EQ(b0101, Calc::add(b1111, b1010));
  EXPECT_EQ(b1010, Calc::add(b1111, b0101));
  EXPECT_EQ(b0000, Calc::add(b1111, b1111));
}

TEST_F(CalcTest, square) {
  EXPECT_EQ(Calc::kGFBits[4], Calc::square(Calc::kGFBits[2]));

  // alpha^10^2 = alpha^20 = alpha^5
  EXPECT_EQ(Calc::kGFBits[5], Calc::square(Calc::kGFBits[10]));
}

TEST_F(CalcTest, Decode) {
  const Calc& calc = calc_errors_;

  const unsigned char s1 = calc.r(1);
  const unsigned char s3 = calc.r(3);
  const unsigned char s5 = calc.r(5);
  ASSERT_EQ(b1011, s1);
  ASSERT_EQ(b1011, s3);
  ASSERT_EQ(b0001, s5);

  const unsigned char s2 = Calc::square(s1);
  const unsigned char s4 = Calc::square(s2);

  // Solve S1+d1=0
  const unsigned char d1 = (~s1) & 0xf;

  // const unsigned char s2d1 = Calc::mult(s2, d1);
  // const unsigned char s4d1 = Calc::mult(s4, d1);
  // const unsigned char s3_plus_s2d1 = Calc::add(s3, s2d1);

  // auto calc_eq2 = [&](const Calc& calc, unsigned char d2, unsigned char d3) {
  //   unsigned char res = s3_plus_s2d1;
  //   res = Calc::add(res, Calc::mult(s1, d2));
  //   res = Calc::add(res, d3);
  //   return res;
  // };
}

}  // namespace
