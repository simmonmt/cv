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
  virtual unsigned char Exp(unsigned char x, int y) const = 0;
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
  unsigned char Exp(unsigned char x, int y) const override;

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

#endif  // _QRCODE_GF_H_
