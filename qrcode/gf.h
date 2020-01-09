#ifndef _QRCODE_GF_H_
#define _QRCODE_GF_H_ 1

#include <initializer_list>
#include <vector>

class GF {
 public:
  virtual ~GF() = default;

  // The returned vector contains powers [0..2^m-2] of alpha, which
  // are members of the field. These elements have m terms, and take
  // the form:
  //
  //    a + b*alpha + c*alpha^2 + d*alpha^3 + ...
  //
  // for values of a, b, c, d in GF(2) (or 0, 1). The elements are
  // stored bitwise in the array as d, c, b, a. The array wraps,
  // because alpha^(2^m-1) == alpha^0.
  //
  // Examples in GF(2^4):
  //   i=0 is alpha^0 => 1,       stored as 0b0001
  //   i=1 is alpha^1 => alpha,   stored as 0b0010
  //   ...
  //   i=3 is alpha^3 => alpha^3, stored as 0b1000
  //   i=4 is alpha^4 => alpha+1, stored as 0b0011
  //   ...
  virtual const std::vector<unsigned char>& PowersOfAlpha() const = 0;

  // This vector contains all elements of the field. The first 2^m-1 are the
  // same as in PowersOfAlpha.
  virtual const std::vector<unsigned char>& Elements() const = 0;

  // Adds elements of the field.
  virtual unsigned char Add(
      std::initializer_list<unsigned char> elems) const = 0;

  // Multiplies elements of the field.
  virtual unsigned char Mult(unsigned char m1, unsigned char m2) const = 0;

  // Returns x^y, where x is an element of the field.
  virtual unsigned char Pow(unsigned char x, int y) const = 0;

  // Returns a specified power of alpha.
  virtual unsigned char AlphaPow(int y) const = 0;
};

// Functions used to perform operations in GF(16), aka GF(2^4).
// See also https://en.wikipedia.org/wiki/Finite_field#GF(16)
class GF16 : public GF {
 public:
  ~GF16() override = default;

  const std::vector<unsigned char>& PowersOfAlpha() const override;
  const std::vector<unsigned char>& Elements() const override;

  unsigned char Add(std::initializer_list<unsigned char> elems) const override;
  unsigned char Mult(unsigned char m1, unsigned char m2) const override;
  unsigned char Pow(unsigned char x, int y) const override;
  unsigned char AlphaPow(int y) const override;

 private:
  static constexpr unsigned char kPowersOfAlpha[15] = {
      1, 2, 4, 8, 3, 6, 12, 11, 5, 10, 7, 14, 15, 13, 9,
  };

  static constexpr unsigned char kElements[16] = {
      1, 2, 4, 8, 3, 6, 12, 11, 5, 10, 7, 14, 15, 13, 9, 0,
  };

  // This array maps the elements that correspond to powers of alpha
  // back to the powers of alpha. The following relationship holds between this
  // array and kPowersOfAlpha:
  //
  //   kElementsToPowers[kPowersOfAlpha[x]] = x
  //
  // NOTE: This array contains a sentinel value at index 0 because there is no
  // power of alpha equal to zero.
  static constexpr unsigned char kElementsToPowers[16] = {
      255, 0, 1, 4, 2, 8, 5, 10, 3, 14, 9, 7, 6, 13, 11, 12,
  };
};

// Functions used to perform operations in GF(256), aka GF(2^8).
class GF256 : public GF {
 public:
  ~GF256() override = default;

  const std::vector<unsigned char>& PowersOfAlpha() const override;
  const std::vector<unsigned char>& Elements() const override;

  unsigned char Add(std::initializer_list<unsigned char> elems) const override;
  unsigned char Mult(unsigned char m1, unsigned char m2) const override;
  unsigned char Pow(unsigned char x, int y) const override;
  unsigned char AlphaPow(int y) const override;

 private:
  static constexpr unsigned char kPowersOfAlpha[255] = {
      1,   2,   4,   8,   16,  32,  64,  128, 29,  58,  116, 232, 205, 135, 19,
      38,  76,  152, 45,  90,  180, 117, 234, 201, 143, 3,   6,   12,  24,  48,
      96,  192, 157, 39,  78,  156, 37,  74,  148, 53,  106, 212, 181, 119, 238,
      193, 159, 35,  70,  140, 5,   10,  20,  40,  80,  160, 93,  186, 105, 210,
      185, 111, 222, 161, 95,  190, 97,  194, 153, 47,  94,  188, 101, 202, 137,
      15,  30,  60,  120, 240, 253, 231, 211, 187, 107, 214, 177, 127, 254, 225,
      223, 163, 91,  182, 113, 226, 217, 175, 67,  134, 17,  34,  68,  136, 13,
      26,  52,  104, 208, 189, 103, 206, 129, 31,  62,  124, 248, 237, 199, 147,
      59,  118, 236, 197, 151, 51,  102, 204, 133, 23,  46,  92,  184, 109, 218,
      169, 79,  158, 33,  66,  132, 21,  42,  84,  168, 77,  154, 41,  82,  164,
      85,  170, 73,  146, 57,  114, 228, 213, 183, 115, 230, 209, 191, 99,  198,
      145, 63,  126, 252, 229, 215, 179, 123, 246, 241, 255, 227, 219, 171, 75,
      150, 49,  98,  196, 149, 55,  110, 220, 165, 87,  174, 65,  130, 25,  50,
      100, 200, 141, 7,   14,  28,  56,  112, 224, 221, 167, 83,  166, 81,  162,
      89,  178, 121, 242, 249, 239, 195, 155, 43,  86,  172, 69,  138, 9,   18,
      36,  72,  144, 61,  122, 244, 245, 247, 243, 251, 235, 203, 139, 11,  22,
      44,  88,  176, 125, 250, 233, 207, 131, 27,  54,  108, 216, 173, 71,  142,
  };

  static constexpr unsigned char kElements[256] = {
      1,   2,   4,   8,   16,  32,  64,  128, 29,  58,  116, 232, 205, 135, 19,
      38,  76,  152, 45,  90,  180, 117, 234, 201, 143, 3,   6,   12,  24,  48,
      96,  192, 157, 39,  78,  156, 37,  74,  148, 53,  106, 212, 181, 119, 238,
      193, 159, 35,  70,  140, 5,   10,  20,  40,  80,  160, 93,  186, 105, 210,
      185, 111, 222, 161, 95,  190, 97,  194, 153, 47,  94,  188, 101, 202, 137,
      15,  30,  60,  120, 240, 253, 231, 211, 187, 107, 214, 177, 127, 254, 225,
      223, 163, 91,  182, 113, 226, 217, 175, 67,  134, 17,  34,  68,  136, 13,
      26,  52,  104, 208, 189, 103, 206, 129, 31,  62,  124, 248, 237, 199, 147,
      59,  118, 236, 197, 151, 51,  102, 204, 133, 23,  46,  92,  184, 109, 218,
      169, 79,  158, 33,  66,  132, 21,  42,  84,  168, 77,  154, 41,  82,  164,
      85,  170, 73,  146, 57,  114, 228, 213, 183, 115, 230, 209, 191, 99,  198,
      145, 63,  126, 252, 229, 215, 179, 123, 246, 241, 255, 227, 219, 171, 75,
      150, 49,  98,  196, 149, 55,  110, 220, 165, 87,  174, 65,  130, 25,  50,
      100, 200, 141, 7,   14,  28,  56,  112, 224, 221, 167, 83,  166, 81,  162,
      89,  178, 121, 242, 249, 239, 195, 155, 43,  86,  172, 69,  138, 9,   18,
      36,  72,  144, 61,  122, 244, 245, 247, 243, 251, 235, 203, 139, 11,  22,
      44,  88,  176, 125, 250, 233, 207, 131, 27,  54,  108, 216, 173, 71,  142,
      0,
  };

  // This array maps the elements that correspond to powers of alpha
  // back to the powers of alpha. The following relationship holds between this
  // array and kPowersOfAlpha:
  //
  //   kElementsToPowers[kPowersOfAlpha[x]] = x
  //
  // NOTE: This array contains a sentinel value at index 0 because there is no
  // power of alpha equal to zero. This particular sentinel makes more sense for
  // fields <2^8 when 255 isn't a valid member of the field.
  static constexpr unsigned char kElementsToPowers[256] = {
      255, 0,   1,   25,  2,   50,  26,  198, 3,   223, 51,  238, 27,  104, 199,
      75,  4,   100, 224, 14,  52,  141, 239, 129, 28,  193, 105, 248, 200, 8,
      76,  113, 5,   138, 101, 47,  225, 36,  15,  33,  53,  147, 142, 218, 240,
      18,  130, 69,  29,  181, 194, 125, 106, 39,  249, 185, 201, 154, 9,   120,
      77,  228, 114, 166, 6,   191, 139, 98,  102, 221, 48,  253, 226, 152, 37,
      179, 16,  145, 34,  136, 54,  208, 148, 206, 143, 150, 219, 189, 241, 210,
      19,  92,  131, 56,  70,  64,  30,  66,  182, 163, 195, 72,  126, 110, 107,
      58,  40,  84,  250, 133, 186, 61,  202, 94,  155, 159, 10,  21,  121, 43,
      78,  212, 229, 172, 115, 243, 167, 87,  7,   112, 192, 247, 140, 128, 99,
      13,  103, 74,  222, 237, 49,  197, 254, 24,  227, 165, 153, 119, 38,  184,
      180, 124, 17,  68,  146, 217, 35,  32,  137, 46,  55,  63,  209, 91,  149,
      188, 207, 205, 144, 135, 151, 178, 220, 252, 190, 97,  242, 86,  211, 171,
      20,  42,  93,  158, 132, 60,  57,  83,  71,  109, 65,  162, 31,  45,  67,
      216, 183, 123, 164, 118, 196, 23,  73,  236, 127, 12,  111, 246, 108, 161,
      59,  82,  41,  157, 85,  170, 251, 96,  134, 177, 187, 204, 62,  90,  203,
      89,  95,  176, 156, 169, 160, 81,  11,  245, 22,  235, 122, 117, 44,  215,
      79,  174, 213, 233, 230, 231, 173, 232, 116, 214, 244, 234, 168, 80,  88,
      175,
  };
};

#endif  // _QRCODE_GF_H_
