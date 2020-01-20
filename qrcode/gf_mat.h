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

  std::vector<unsigned char> Row(int row);

  std::unique_ptr<GFMat> Mult(const GFMat& other) const;

  void Print() const;

 protected:
  const GF& gf() const { return gf_; }

  virtual void set_dirty() {}

 private:
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

  unsigned char Determinant() const;
  std::unique_ptr<GFSqMat> Inverse() const;

 private:
  void set_dirty() { det_.reset(); }

  unsigned char CalculateDeterminant() const;

  mutable absl::optional<unsigned char> det_;
};

#endif  // _QRCODE_GF_MAT_H_
