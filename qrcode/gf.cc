#include "qrcode/gf.h"

#include "absl/base/macros.h"

constexpr unsigned char GF16::kPowersOfAlpha[];
constexpr unsigned char GF16::kElements[];
constexpr unsigned char GF16::kElementsToPowers[];

const std::vector<unsigned char>& GF16::PowersOfAlpha() const {
  static const std::vector<unsigned char> kVec(
      kPowersOfAlpha, kPowersOfAlpha + ABSL_ARRAYSIZE(kPowersOfAlpha));
  return kVec;
}

const std::vector<unsigned char>& GF16::Elements() const {
  static const std::vector<unsigned char> kVec(
      kElements, kElements + ABSL_ARRAYSIZE(kElements));
  return kVec;
}

unsigned char GF16::Add(std::initializer_list<unsigned char> elems) const {
  // Addition is defined as bitwise XOR.
  auto iter = elems.begin();
  if (iter == elems.end()) {
    return 0;
  }
  unsigned char res = *iter;
  for (++iter; iter != elems.end(); ++iter) {
    res = res ^ *iter;
  }
  return res;
}

unsigned char GF16::Mult(unsigned char m1, unsigned char m2) const {
  const unsigned char a = m1 & 1, b = m1 & 2;
  const unsigned char c = m1 & 4, d = m1 & 8;
  const unsigned char e = m2 & 1, f = m2 & 2;
  const unsigned char g = m2 & 4, h = m2 & 8;

  // Given two polynomials in GF(2^4)
  //   a + b*alpha + c*alpha^2 + d*alpha^3
  //   e + f*alpha + g*alpha^2 + h*alpha^3
  // multiplication is defined as:
  //   (ae+bh+cg+df) +
  //   (af+be+bh+cg+df+ch+dg) * alpha +
  //   (ag+bf+ce+ch+dg+dh) * alpha^2 +
  //   (ah+bg+cf+de+dh) * alpha^3
  // Source: https://en.wikipedia.org/wiki/Finite_field#GF(16)
  //
  // Operations on the coefficients (i.e. a-h) are performed in
  // GF(2). GF(2) addition and multiplication are bitwise XOR and
  // AND, respectively.

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

unsigned char GF16::Exp(unsigned char x, int y) const {
  if (x == 0) {
    return x;
  }
  return kPowersOfAlpha[(kElementsToPowers[x] * y) % 15];
}

unsigned char GF16::Power(int y) const { return kPowersOfAlpha[y % 15]; }

constexpr unsigned char GF32::kPowersOfAlpha[];
constexpr unsigned char GF32::kElementsToPowers[];
constexpr unsigned char GF32::kElements[];

const std::vector<unsigned char>& GF32::PowersOfAlpha() const {
  static const std::vector<unsigned char> kVec(
      kPowersOfAlpha, kPowersOfAlpha + ABSL_ARRAYSIZE(kPowersOfAlpha));
  return kVec;
}

const std::vector<unsigned char>& GF32::Elements() const {
  static const std::vector<unsigned char> kVec(
      kElements, kElements + ABSL_ARRAYSIZE(kElements));
  return kVec;
}

unsigned char GF32::Add(std::initializer_list<unsigned char> elems) const {
  // Addition is defined as bitwise XOR.
  auto iter = elems.begin();
  if (iter == elems.end()) {
    return 0;
  }
  unsigned char res = *iter;
  for (++iter; iter != elems.end(); ++iter) {
    res = res ^ *iter;
  }
  return res;
}

unsigned char GF32::Mult(unsigned char m1, unsigned char m2) const {
  const unsigned char a = m1 & 1, b = m1 & 2, c = m1 & 4;
  const unsigned char d = m1 & 8, e = m1 & 16;
  const unsigned char f = m2 & 1, g = m2 & 2, h = m2 & 4;
  const unsigned char i = m2 & 8, j = m2 & 16;

  // Given two polynomials in GF(2^5)
  //   a + b*alpha + c*alpha^2 + d*alpha^3 + e*alpha^4
  //   f + g*alpha + h*alpha^2 + i*alpha^3 + j*alpha^4
  // multiplication is defined as
  //   (af+bj+ci+dh+eg+ej) +
  //   (ag+bf+cj+di+eh) * alpha +
  //   (ah+bg+bj+cf+ci+dh+dj+eg+ei+ej) * alpha^2 +
  //   (ai+bh+cg+cj+df+di+eh+ej) * alpha^3 +
  //   (aj+bi+ch+dg+dj+ef+ei) * alpha^4
  // Found by polynomial multiplication and simplification.
  unsigned char out_a =
      (a && f) ^ (b && j) ^ (c && i) ^ (d && h) ^ (e && g) ^ (e && j);
  unsigned char out_b = (a && g) ^ (b && f) ^ (c && j) ^ (d && i) ^ (e && h);
  unsigned char out_c = (a && h) ^ (b && g) ^ (b && j) ^ (c && f) ^ (c && i) ^
                        (d && h) ^ (d && j) ^ (e && g) ^ (e && i) ^ (e && j);
  unsigned char out_d = (a && i) ^ (b && h) ^ (c && g) ^ (c && j) ^ (d && f) ^
                        (d && i) ^ (e && h) ^ (e && j);
  unsigned char out_e = (a && j) ^ (b && i) ^ (c && h) ^ (d && g) ^ (d && j) ^
                        (e && f) ^ (e && i);

  return ((out_e & 1) << 4) | ((out_d & 1) << 3) | ((out_c & 1) << 2) |
         ((out_b & 1) << 1) | (out_a & 1);
}

unsigned char GF32::Exp(unsigned char x, int y) const {
  if (x == 0) {
    return x;
  }
  return kPowersOfAlpha[(kElementsToPowers[x] * y) %
                        ABSL_ARRAYSIZE(kPowersOfAlpha)];
}

unsigned char GF32::Power(int y) const {
  return kPowersOfAlpha[y % ABSL_ARRAYSIZE(kPowersOfAlpha)];
}
