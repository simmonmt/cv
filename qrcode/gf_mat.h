// A matrix library tailored for use with GF(2^x).

#ifndef _QRCODE_GF_MAT_H_
#define _QRCODE_GF_MAT_H_ 1

#include "qrcode/gf.h"

#include "absl/types/optional.h"

class GFMat {
 public:
  GFMat(const GF& gf, int h, int w)
      : gf_(gf), h_(h), w_(w), arr_(new unsigned char[w * h]) {}
  virtual ~GFMat() = default;

  int w() const { return w_; }
  int h() const { return h_; }

  unsigned char Get(int row, int col) const { return arr_[row * w_ + col]; }
  void Set(int row, int col, unsigned char v) {
    set_dirty();
    arr_[row * w_ + col] = v;
  }

  // Return the contents of a given row.
  std::vector<unsigned char> Row(int row);

  // Multiply two matrices together as this * other, returning the result in a
  // new matrix. Note that the result is a GFMat even if the dimensions are such
  // that it could be a GFSqMat. Returns nullptr if the matrices cannot be
  // multiplied against each other.
  std::unique_ptr<GFMat> Mult(const GFMat& other) const;

  // Dumps the contents of the matrix to cout.
  void Print() const;

 protected:
  const GF& gf() const { return gf_; }

  virtual void set_dirty() {}

 private:
  // Computes the dot product of the given row from this matrix and the given
  // column from other.
  unsigned char Dot(int row, const GFMat& other, int col) const;

  const GF& gf_;
  const int h_, w_;
  unsigned char* arr_;
};

// A square matrix
class GFSqMat : public GFMat {
 public:
  GFSqMat(const GF& gf, int sz) : GFMat(gf, sz, sz) {}

  int sz() const { return w(); }

  // Computes the determinant of this matrix.
  unsigned char Determinant() const;

  // Returns the inverse of this matrix. Returns nullptr if the matrix is not
  // invertible.
  std::unique_ptr<GFSqMat> Inverse() const;

 private:
  // Reset cached values whenever we change the contents of the matrix.
  void set_dirty() { det_.reset(); }

  unsigned char CalculateDeterminant() const;

  // Mutable because we cache the determinant, which is expensive to calculate.
  mutable absl::optional<unsigned char> det_;
};

#endif  // _QRCODE_GF_MAT_H_
