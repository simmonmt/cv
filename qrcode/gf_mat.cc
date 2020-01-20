#include "qrcode/gf_mat.h"

#include <bitset>
#include <iostream>

#include "absl/memory/memory.h"

void GFMat::Load(const std::vector<std::vector<unsigned char>>& in) {
  for (int row = 0; row < in.size(); ++row) {
    for (int col = 0; col < in[row].size(); ++col) {
      Set(row, col, in[row][col]);
    }
  }
}

std::vector<unsigned char> GFMat::Row(int row) {
  std::vector<unsigned char> out(w_);
  for (int i = 0; i < w_; ++i) {
    out[i] = Get(row, i);
  }
  return out;
}

void GFMat::Print() const {
  for (int row = 0; row < h(); ++row) {
    for (int col = 0; col < w(); ++col) {
      std::cout << " " << std::bitset<8>(Get(row, col));
    }
    std::cout << "\n";
  }
}

std::unique_ptr<GFMat> GFMat::Mult(const GFMat& other) const {
  if (w() != other.h()) {
    return nullptr;
  }

  auto out = absl::make_unique<GFMat>(gf(), other.w(), h());
  for (int row = 0; row < h(); ++row) {
    for (int col = 0; col < other.w(); ++col) {
      out->Set(row, col, Dot(row, other, col));
    }
  }
  return out;
}

unsigned char GFMat::Dot(int row, const GFMat& other, int col) const {
  // assert this->w() == other.h()

  unsigned char result = 0;
  for (int i = 0; i < w(); ++i) {
    const unsigned char a = Get(row, i);
    const unsigned char b = other.Get(i, col);
    result = gf().Add({result, gf().Mult(a, b)});
  }
  return result;
}

unsigned char GFSqMat::Determinant() const {
  if (!det_.has_value()) {
    det_ = CalculateDeterminant();
  }
  return *det_;
}

unsigned char GFSqMat::CalculateDeterminant() const {
  if (sz() == 1) {
    return Get(0, 0);
  } else if (sz() == 2) {
    return gf().Sub(
        {gf().Mult(Get(0, 0), Get(1, 1)), gf().Mult(Get(1, 0), Get(0, 1))});
  }

  unsigned char total = 0;
  GFSqMat sub(gf(), sz() - 1);
  for (int i = 0; i < sz(); ++i) {
    for (int row = 1; row < sz(); ++row) {
      int sub_col = 0;
      for (int col = 0; col < sz(); ++col) {
        if (col == i) {
          continue;
        }

        sub.Set(row - 1, sub_col, Get(row, col));
        sub_col++;
      }
    }

    unsigned char d = gf().Mult(Get(0, i), sub.Determinant());

    if (i % 2 == 0) {
      total = gf().Add({total, d});
    } else {
      total = gf().Sub({total, d});
    }
  }

  return total;
}

std::unique_ptr<GFSqMat> GFSqMat::Inverse() const {
  auto out = absl::make_unique<GFSqMat>(gf(), sz());

  if (sz() == 1) {
    out->Set(0, 0, gf().Inverse(Get(0, 0)));
    return out;
  }

  if (sz() == 2) {
    out->Set(0, 0, Get(1, 1));
    out->Set(1, 1, Get(0, 0));
    out->Set(1, 0, Get(1, 0));
    out->Set(0, 1, Get(0, 1));

    unsigned char invdet = gf().Inverse(Determinant());

    for (int y = 0; y < out->sz(); ++y) {
      for (int x = 0; x < out->sz(); ++x) {
        out->Set(x, y, gf().Mult(invdet, out->Get(x, y)));
      }
    }

    return out;
  }

  return nullptr;
}
