#include "qrcode/qr_decode_utils.h"

#include <vector>

#include "absl/base/macros.h"
#include "absl/strings/str_format.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {

using ::testing::ElementsAre;

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

  static unsigned char add(std::initializer_list<unsigned char> elems) {
    auto iter = elems.begin();
    if (iter == elems.end()) {
      return 0;
    }
    unsigned char res = *iter;
    for (++iter; iter != elems.end(); ++iter) {
      res = add(res, *iter);
    }
    return res;
  }

  static unsigned char pow(unsigned char a, int exp) {
    return kGFBits[(kGFIndexes[a] * exp) % 15];
  }

  static unsigned char mult(unsigned char m1, unsigned char m2) {
    const unsigned char a = m1 & 1, b = m1 & 2;
    const unsigned char c = m1 & 4, d = m1 & 8;
    const unsigned char e = m2 & 1, f = m2 & 2;
    const unsigned char g = m2 & 4, h = m2 & 8;

    // From https://en.wikipedia.org/wiki/Finite_field#GF(16) :
    //   (ae+bh+cg+df) +
    //   (af+be+bh+cg+df+ch+dg) * alpha +
    //   (ag+bf+ce+ch+dg+dh) * alpha^2 +
    //   (ah+bg+cf+de+dh) * alpha^3

    const unsigned char out_a = (a && e) ^ (b && h) ^ (c && g) ^ (d && f);
    const unsigned char out_b = (a && f) ^ (b && e) ^ (b && h) ^ (c && g) ^
                                (d && f) ^ (c && h) ^ (d && g);
    const unsigned char out_c =
        (a && g) ^ (b && f) ^ (c && e) ^ (c && h) ^ (d && g) ^ (d && h);
    const unsigned char out_d =
        (a && h) ^ (b && g) ^ (c && f) ^ (d && e) ^ (d && h);

    return ((out_d & 1) << 3) | ((out_c & 1) << 2) | ((out_b & 1) << 1) |
           (out_a & 1);
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
      : calc_errors_(
            // Has errors at *bit indexes* 5 and 13. Note these vectors are
            // MSB-first.
            {1, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 1, 0, 0}),
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

TEST_F(CalcTest, Add) {
  EXPECT_EQ(b0000, Calc::add(b0000, b0000));
  EXPECT_EQ(b1111, Calc::add(b0101, b1010));
  EXPECT_EQ(b0101, Calc::add(b1111, b1010));
  EXPECT_EQ(b1010, Calc::add(b1111, b0101));
  EXPECT_EQ(b0000, Calc::add(b1111, b1111));
}

TEST_F(CalcTest, Pow) {
  EXPECT_EQ(Calc::kGFBits[4], Calc::pow(Calc::kGFBits[2], 2));

  // alpha^10^2 = alpha^20 = alpha^5
  EXPECT_EQ(Calc::kGFBits[5], Calc::pow(Calc::kGFBits[10], 2));
}

TEST_F(CalcTest, Mult) {
  for (int i = 0; i < ABSL_ARRAYSIZE(Calc::kGFBits); ++i) {
    for (int j = 0; j < ABSL_ARRAYSIZE(Calc::kGFBits); ++j) {
      const unsigned char m1 = Calc::kGFBits[i];
      const unsigned char m2 = Calc::kGFBits[j];
      const unsigned char res = Calc::kGFBits[(i + j) % 15];

      EXPECT_EQ(res, Calc::mult(m1, m2))
          << "i=" << i << ",j=" << j << " " << int(m1) << "*" << int(m2) << "="
          << int(res);

      if (i == j) {
        EXPECT_EQ(res, Calc::pow(m1, 2)) << "square " << i << " " << int(m1);
      }
    }
  }
}

TEST_F(CalcTest, Decode) {
  const Calc& calc = calc_errors_;

  const unsigned char s1 = calc.r(1);
  const unsigned char s3 = calc.r(3);
  const unsigned char s5 = calc.r(5);
  ASSERT_EQ(b1011, s1);
  ASSERT_EQ(b1011, s3);
  ASSERT_EQ(b0001, s5);

  const unsigned char s2 = Calc::pow(s1, 2);
  const unsigned char s4 = Calc::pow(s2, 2);

  // Solve Eq1: S1 + d1 = 0
  const unsigned char d1 = s1;  // (~s1) & 0xf;

  // Verify Eq1
  ASSERT_EQ(0, Calc::add(s1, d1));

  // Eq2: S3 + S2*d1 + S1*d2 + d3 = 0
  const unsigned char s3_plus_s2d1 = Calc::add(s3, Calc::mult(s2, d1));
  auto eq2 = [&](unsigned char d2, unsigned char d3) {
    unsigned char res = s3_plus_s2d1;
    res = Calc::add(res, Calc::mult(s1, d2));
    res = Calc::add(res, d3);
    return res;
  };

  // Eq3: S5 + S4*d1 + S3*d2 + S2*d3 = 0
  const unsigned char s5_plus_s4d1 = Calc::add(s5, Calc::mult(s4, d1));
  auto eq3 = [&](unsigned char d2, unsigned char d3) {
    unsigned char res = s5_plus_s4d1;
    res = Calc::add(res, Calc::mult(s3, d2));
    res = Calc::add(res, Calc::mult(s2, d3));
    return res;
  };

  static constexpr unsigned char kGFBitsWithZero[] = {
      1, 2, 4, 8, 3, 6, 12, 11, 5, 10, 7, 14, 15, 13, 9, 0,
  };

  bool found = false;
  unsigned char d2, d3;
  for (int i = 0; !found && i < ABSL_ARRAYSIZE(kGFBitsWithZero); ++i) {
    for (int j = 0; !found && j < ABSL_ARRAYSIZE(kGFBitsWithZero); ++j) {
      d2 = Calc::kGFBits[i];
      d3 = Calc::kGFBits[j];
      if (eq2(d2, d3) == 0 && eq3(d2, d3) == 0) {
        found = true;
      }
    }
  }
  ASSERT_TRUE(found);

  // Verify eq2 and eq3
  ASSERT_EQ(0, Calc::add({s3,                  //
                          Calc::mult(s2, d1),  //
                          Calc::mult(s1, d2),  //
                          d3}));
  ASSERT_EQ(0, Calc::add({s5,                  //
                          Calc::mult(s4, d1),  //
                          Calc::mult(s3, d2),  //
                          Calc::mult(s2, d3)}));

  std::vector<int> errors;
  for (int i = 0; i < ABSL_ARRAYSIZE(Calc::kGFBits); ++i) {
    // Look for x^3 + d1*x^2 + d2*x + d3 == 0
    unsigned char x = Calc::kGFBits[i];
    unsigned char res = Calc::add({Calc::pow(x, 3),                  //
                                   Calc::mult(d1, Calc::pow(x, 2)),  //
                                   Calc::mult(d2, x),                //
                                   d3});

    if (res == 0) {
      errors.push_back(i);
    }
  }

  EXPECT_THAT(errors, ElementsAre(5, 13));
}

}  // namespace
