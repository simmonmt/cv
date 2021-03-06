#include "qrcode/gf.h"

#include <bitset>
#include <sstream>

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

unsigned char GF16::Inverse(unsigned char x) const { return kInverse[x]; }

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

const unsigned char GF16::kInverse[16] = {0,  1, 9,  14, 13, 11, 7, 6,
                                          15, 2, 12, 5,  10, 4,  3, 8};

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

unsigned char GF256::Inverse(unsigned char x) const { return kInverse[x]; }

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

const unsigned char GF256::kInverse[256] = {
    0,   1,   142, 244, 71,  167, 122, 186, 173, 157, 221, 152, 61,  170, 93,
    150, 216, 114, 192, 88,  224, 62,  76,  102, 144, 222, 85,  128, 160, 131,
    75,  42,  108, 237, 57,  81,  96,  86,  44,  138, 112, 208, 31,  74,  38,
    139, 51,  110, 72,  137, 111, 46,  164, 195, 64,  94,  80,  34,  207, 169,
    171, 12,  21,  225, 54,  95,  248, 213, 146, 78,  166, 4,   48,  136, 43,
    30,  22,  103, 69,  147, 56,  35,  104, 140, 129, 26,  37,  97,  19,  193,
    203, 99,  151, 14,  55,  65,  36,  87,  202, 91,  185, 196, 23,  77,  82,
    141, 239, 179, 32,  236, 47,  50,  40,  209, 17,  217, 233, 251, 218, 121,
    219, 119, 6,   187, 132, 205, 254, 252, 27,  84,  161, 29,  124, 204, 228,
    176, 73,  49,  39,  45,  83,  105, 2,   245, 24,  223, 68,  79,  155, 188,
    15,  92,  11,  220, 189, 148, 172, 9,   199, 162, 28,  130, 159, 198, 52,
    194, 70,  5,   206, 59,  13,  60,  156, 8,   190, 183, 135, 229, 238, 107,
    235, 242, 191, 175, 197, 100, 7,   123, 149, 154, 174, 182, 18,  89,  165,
    53,  101, 184, 163, 158, 210, 247, 98,  90,  133, 125, 168, 58,  41,  113,
    200, 246, 249, 67,  215, 214, 16,  115, 118, 120, 153, 10,  25,  145, 20,
    63,  230, 240, 134, 177, 226, 241, 250, 116, 243, 180, 109, 33,  178, 106,
    227, 231, 181, 234, 3,   143, 211, 201, 66,  212, 232, 117, 127, 255, 126,
    253,
};

std::string VecToString(const GF& gf, const std::vector<unsigned char>& vec) {
  std::stringstream ss;
  for (const auto& elem : vec) {
    if (ss.tellp() != 0) {
      ss << " ";
    }
    ss << std::bitset<8>(elem);
  }
  return ss.str();
}
