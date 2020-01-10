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

unsigned char GF16::Pow(unsigned char x, int y) const {
  if (y == 0) {
    return 1;
  } else if (x == 0) {
    return 0;
  }
  return kPowersOfAlpha[(kElementsToPowers[x] * y) % 15];
}

unsigned char GF16::AlphaPow(int y) const { return kPowersOfAlpha[y % 15]; }

int GF16::ToAlphaPow(unsigned char x) const { return kElementsToPowers[x]; }

constexpr unsigned char GF256::kPowersOfAlpha[];
constexpr unsigned char GF256::kElements[];
constexpr unsigned char GF256::kElementsToPowers[];

const std::vector<unsigned char>& GF256::PowersOfAlpha() const {
  static const std::vector<unsigned char> kVec(
      kPowersOfAlpha, kPowersOfAlpha + ABSL_ARRAYSIZE(kPowersOfAlpha));
  return kVec;
}

const std::vector<unsigned char>& GF256::Elements() const {
  static const std::vector<unsigned char> kVec(
      kElements, kElements + ABSL_ARRAYSIZE(kElements));
  return kVec;
}

unsigned char GF256::Add(std::initializer_list<unsigned char> elems) const {
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

unsigned char GF256::Mult(unsigned char m1, unsigned char m2) const {
  const unsigned char a = m1 & 1, i = m2 & 1;
  const unsigned char b = m1 & 2, j = m2 & 2;
  const unsigned char c = m1 & 4, k = m2 & 4;
  const unsigned char d = m1 & 8, l = m2 & 8;
  const unsigned char e = m1 & 16, m = m2 & 16;
  const unsigned char f = m1 & 32, n = m2 & 32;
  const unsigned char g = m1 & 64, o = m2 & 64;
  const unsigned char h = m1 & 128, p = m2 & 128;

  unsigned char out_a = (a && i) ^ (b && p) ^ (c && o) ^ (d && n) ^ (e && m) ^
                        (f && l) ^ (f && p) ^ (g && k) ^ (g && o) ^ (g && p) ^
                        (h && j) ^ (h && n) ^ (h && o) ^ (h && p);
  unsigned char out_b = (a && j) ^ (b && i) ^ (c && p) ^ (d && o) ^ (e && n) ^
                        (f && m) ^ (g && l) ^ (g && p) ^ (h && k) ^ (h && o) ^
                        (h && p);
  unsigned char out_c = (a && k) ^ (b && j) ^ (b && p) ^ (c && i) ^ (c && o) ^
                        (d && n) ^ (d && p) ^ (e && m) ^ (e && o) ^ (f && l) ^
                        (f && n) ^ (f && p) ^ (g && k) ^ (g && m) ^ (g && o) ^
                        (g && p) ^ (h && j) ^ (h && l) ^ (h && n) ^ (h && o);
  unsigned char out_d = (a && l) ^ (b && k) ^ (b && p) ^ (c && j) ^ (c && o) ^
                        (c && p) ^ (d && i) ^ (d && n) ^ (d && o) ^ (e && m) ^
                        (e && n) ^ (e && p) ^ (f && l) ^ (f && m) ^ (f && o) ^
                        (f && p) ^ (g && k) ^ (g && l) ^ (g && n) ^ (g && o) ^
                        (h && j) ^ (h && k) ^ (h && m) ^ (h && n);
  unsigned char out_e = (a && m) ^ (b && l) ^ (b && p) ^ (c && k) ^ (c && o) ^
                        (c && p) ^ (d && j) ^ (d && n) ^ (d && o) ^ (d && p) ^
                        (e && i) ^ (e && m) ^ (e && n) ^ (e && o) ^ (f && l) ^
                        (f && m) ^ (f && n) ^ (g && k) ^ (g && l) ^ (g && m) ^
                        (h && j) ^ (h && k) ^ (h && l) ^ (h && p);
  unsigned char out_f = (a && n) ^ (b && m) ^ (c && l) ^ (c && p) ^ (d && k) ^
                        (d && o) ^ (d && p) ^ (e && j) ^ (e && n) ^ (e && o) ^
                        (e && p) ^ (f && i) ^ (f && m) ^ (f && n) ^ (f && o) ^
                        (g && l) ^ (g && m) ^ (g && n) ^ (h && k) ^ (h && l) ^
                        (h && m);
  unsigned char out_g = (a && o) ^ (b && n) ^ (c && m) ^ (d && l) ^ (d && p) ^
                        (e && k) ^ (e && o) ^ (e && p) ^ (f && j) ^ (f && n) ^
                        (f && o) ^ (f && p) ^ (g && i) ^ (g && m) ^ (g && n) ^
                        (g && o) ^ (h && l) ^ (h && m) ^ (h && n);
  unsigned char out_h = (a && p) ^ (b && o) ^ (c && n) ^ (d && m) ^ (e && l) ^
                        (e && p) ^ (f && k) ^ (f && o) ^ (f && p) ^ (g && j) ^
                        (g && n) ^ (g && o) ^ (g && p) ^ (h && i) ^ (h && m) ^
                        (h && n) ^ (h && o);

  return ((out_h & 1) << 7) | ((out_g & 1) << 6) | ((out_f & 1) << 5) |
         ((out_e & 1) << 4) | ((out_d & 1) << 3) | ((out_c & 1) << 2) |
         ((out_b & 1) << 1) | (out_a & 1);
}

unsigned char GF256::Pow(unsigned char x, int y) const {
  if (y == 0) {
    return 1;
  } else if (x == 0) {
    return 0;
  }
  return kPowersOfAlpha[(kElementsToPowers[x] * y) % 255];
}

unsigned char GF256::AlphaPow(int y) const { return kPowersOfAlpha[y % 255]; }

int GF256::ToAlphaPow(unsigned char x) const { return kElementsToPowers[x]; }
