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
    unsigned char d = gf().Mult(Get(0, i), SubDeterminant(0, i, &sub));

    if (i % 2 == 0) {
      total = gf().Add({total, d});
    } else {
      total = gf().Sub({total, d});
    }
  }

  return total;
}

unsigned char GFSqMat::SubDeterminant(int exclude_row, int exclude_col,
                                      GFSqMat* sub) const {
  int sub_row = 0, sub_col = 0;
  for (int row = 0; row < sz(); ++row) {
    if (row == exclude_row) {
      continue;
    }

    for (int col = 0; col < sz(); ++col) {
      if (col == exclude_col) {
        continue;
      }

      sub->Set(sub_row, sub_col, Get(row, col));
      ++sub_col;
    }

    ++sub_row;
    sub_col = 0;
  }

  return sub->Determinant();
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

  // Invert square matrices larger than 2x2.
  // Source:
  // https://www.mathsisfun.com/algebra/matrix-inverse-minors-cofactors-adjugate.html

  // Step 1: Matrix of minors
  GFSqMat sub(gf(), sz() - 1);
  for (int row = 0; row < sz(); ++row) {
    for (int col = 0; col < sz(); ++col) {
      out->Set(row, col, SubDeterminant(row, col, &sub));
    }
  }

  // Step 2: Matrix of Cofactors
  // We can skip this because negation isn't a thing in GF(2^x)

  // Step 3: Transpose
  // Step 4: Multiply by 1/Determinant
  unsigned char invdet = gf().Inverse(Determinant());
  for (int row = 0; row < sz(); ++row) {
    for (int col = 0; col <= row; ++col) {
      if (row == col) {
        // On-diagonal, so just multiply
        out->Set(row, col, gf().Mult(invdet, out->Get(row, col)));
        continue;
      }

      // Off-diagonal, so transpose and multiply
      unsigned char tmp = out->Get(row, col);
      out->Set(row, col, gf().Mult(invdet, out->Get(col, row)));
      out->Set(col, row, gf().Mult(invdet, tmp));
    }
  }

  return out;
}
