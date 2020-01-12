#include <bitset>
#include <cmath>
#include <iostream>

#include "absl/types/variant.h"
#include "qrcode/stl_logging.h"

#include "qrcode/gf.h"

namespace {

class Mat {
 public:
  Mat(int w, int h) : w_(w), h_(h), arr_(new unsigned char[w * h]) {}
  virtual ~Mat() = default;

  int w() const { return w_; }
  int h() const { return h_; }

  unsigned char Get(int x, int y) const { return arr_[y * w_ + x]; }
  void Set(int x, int y, unsigned char v) { arr_[y * w_ + x] = v; }

  std::vector<unsigned char> Row(int row) {
    std::vector<unsigned char> out(w_);
    for (int i = 0; i < w_; ++i) {
      out[i] = Get(i, row);
    }
    return out;
  }

 private:
  const int w_, h_;
  unsigned char* arr_;
};

// A square matrix
class SqMat : public Mat {
 public:
  SqMat(int sz) : Mat(sz, sz) {}

  int sz() const { return w(); }
};

// Calculate the determinant of a square matrix using GF arithmetic.
unsigned char Determinant(const GF& gf, const SqMat& m) {
  if (m.sz() == 2) {
    return gf.Sub(
        {gf.Mult(m.Get(0, 0), m.Get(1, 1)), gf.Mult(m.Get(0, 1), m.Get(1, 0))});
  }

  unsigned char total = 0;
  SqMat sub(m.sz() - 1);
  for (int i = 0; i < m.sz(); ++i) {
    for (int y = 1; y < m.sz(); ++y) {
      int subx = 0;
      for (int x = 0; x < m.sz(); ++x) {
        if (x == i) {
          continue;
        }

        sub.Set(subx, y - 1, m.Get(x, y));
        subx++;
      }
    }

    unsigned char d = gf.Mult(m.Get(i, 0), Determinant(gf, sub));

    if (i % 2 == 0) {
      total = gf.Add({total, d});
    } else {
      total = gf.Sub({total, d});
    }
  }

  return total;
}

// Evaluate a polynomial described by the coeffs vector. The polynomial looks
// like this:
//
//   f(x) = coeffs_0 * x^0 + coeffs_1 * x^1 + ...
//
// Evaluation uses GF arithmetic.
unsigned char EvalPoly(const GF& gf, const std::vector<unsigned char>& coeffs,
                       const unsigned char x) {
  unsigned char res = 0;
  for (int i = 0; i < coeffs.size(); ++i) {
    res = gf.Add({res, gf.Mult(coeffs[i], gf.Pow(x, i))});
  }
  return res;
}

// Determine which members of `gf` are zeroes for the polynomial specified in
// `coeffs` (see EvalPoly for formatting).
std::vector<unsigned char> FindZeros(const GF& gf,
                                     const std::vector<unsigned char>& coeffs) {
  // We're using small fields, so find the zeros the hard way. If I was feeling
  // adventurous, which I'm not, I could implement a Chien search.
  std::vector<unsigned char> zeros;
  for (const unsigned char& elem : gf.Elements()) {
    if (EvalPoly(gf, coeffs, elem) == 0) {
      zeros.push_back(elem);
    }
  }
  return zeros;
}

// Evaluate the dot product of two vectors using GF arithmetic.
unsigned char DotProduct(const GF& gf, const std::vector<unsigned char>& a,
                         const std::vector<unsigned char>& b) {
  unsigned char res = 0;
  for (int i = 0; i < a.size(); ++i) {
    res = gf.Add({res, gf.Mult(a[i], b[i])});
  }
  return res;
}

// Implements the Peterson-Gorenstein-Zeirler algorithm to calculate the error
// locator polynomial that corresponds to a passed-in set of syndromes. See
// https://en.m.wikipedia.org/wiki/BCH_code#Peterson-Gorenstein-Zierler_algorithm
// for details.
//
// The syndromes vector contains syndromes s_c through at least s_{c+2t-1} at
// indexes corresponding to their subscripts (i.e. s_c is at index c -- not
// index 0).
//
// It returns a set of coefficients lambda_{0..v} that form the following
// polynomial:
//
//   lambda(x) = 1 + lambda_1 * x + lambda_2 * x^2 + ... + lambda_v * x^v
//
// The PGZ algorithm doesn't find lambda_0 (it's assumed to be 1), but we return
// it anyway to ease evaluation of the lambda polynomial.
//
// An empty vector is returned if an error occurs -- if the polynomial cannot be
// derived.
std::vector<unsigned char> PGZ(const GF& gf, int c, int t,
                               const std::vector<unsigned char>& syndromes) {
  int v = t;

  for (;;) {
    // Steps 1 and 2: Make matrices S_{v,v} and C_{v,1}
    SqMat a(v);
    std::vector<unsigned char> cvec(v);
    for (int y = 0; y < v; ++y) {
      for (int x = 0; x < v; ++x) {
        a.Set(x, y, syndromes[c + x + y]);
      }
      cvec[y] = syndromes[c + v + y];
    }

    // There's no code for steps 3 or 4 since they're just defining terms we'll
    // use later, namely that there exists lambda_{v,1} such that
    //
    //   S_{v,v} * lambda_{v,1} = C{v,1}
    //
    // The whole point of this exercise is to find lambda_{v,1} for some v.
    //
    // Note that because we're in a GF(2^x) field addition is the same as
    // subtraction, so PGZ's -C_{v,1} = C_{v,1}.

    unsigned char det = Determinant(gf, a);

    // Step 6: If the determinant is zero, try again with v-- unless it's
    // already zero. If it's zero, we've failed -- there's no solution.
    if (det == 0) {
      if (v == 0) {
        return {};
      } else {
        v--;
        continue;
      }
    }

    // Step 5: The determinant is non-zero, which means we can invert S_{v,v}
    // which in turn means we can solve the above equation for
    // lambda_{v,1}. Inverting S_{v,v} means multiplying it by 1/det(S). We're
    // doing field math, so calculating 1/det(S) means finding the
    // multiplicative inverse of det(S).
    //
    // We're using small fields, and we don't (yet?) precompute the
    // multiplicative inverses of field members, so find the multiplicative
    // inverse of the determinant the hard way.
    unsigned char invdet;
    for (const auto& elem : gf.Elements()) {
      if (gf.Mult(elem, det) == 0b0001) {
        invdet = elem;
        break;
      }
    }

    // Step 5 (cont): Calculate (1/det(S)) * S
    for (int y = 0; y < v; ++y) {
      for (int x = 0; x < v; ++x) {
        a.Set(x, y, gf.Mult(invdet, a.Get(x, y)));
      }
    }

    // Step 5 (cont): Calculate ((1/det(S)) * S) * C, which gives us the error
    // locator polynomial coefficients lambda_1, lambda_2, ..., lambda_v,
    // forming this polynomial:
    //
    // lambda(x) = 1 + lambda_1 * x + lambda_2 * x^2 + ... + lambda_v * x^v
    //
    // The vector `lambda` contains these coefficients in order, starting with a
    // lambda_0=1.
    std::vector<unsigned char> lambda(a.h() + 1);
    lambda[0] = 1;
    for (int i = 0; i < a.h(); ++i) {
      lambda[v - i] = DotProduct(gf, a.Row(i), cvec);
    }

    // Step 7: We're done.
    return lambda;
  }
}

unsigned char R(const GF& gf, const std::vector<bool> poly, int alpha_power) {
  unsigned char out_bits = 0;
  for (int i = 0; i < poly.size(); ++i) {
    if (poly[i]) {
      out_bits = gf.Add({out_bits, gf.AlphaPow(i * alpha_power)});
    }
  }
  return out_bits;
}

}  // namespace

absl::variant<std::vector<bool>, std::string> DecodeBCH(
    const GF& gf, const std::vector<bool>& bits, int c, int d) {
  int s_lo = c;
  int s_hi = c + d - 2;

  std::vector<unsigned char> syndromes(s_hi + 1);

  bool all_zero = true;
  for (int i = s_lo; i <= s_hi; i++) {
    unsigned char r = R(gf, bits, i);
    syndromes[i] = r;
    if (r != 0) {
      all_zero = false;
    }
  }

  if (all_zero) {
    return bits;
  }

  int t = (d - 1) / 2;

  std::vector<unsigned char> lambda = PGZ(gf, c, t, syndromes);
  if (lambda.empty()) {
    return "no lambda found";
  }

  std::vector<unsigned char> zeros = FindZeros(gf, lambda);
  if (zeros.empty()) {
    return "no zeros found";
  }

  std::vector<bool> out = bits;
  int lim = std::pow(2, gf.m()) - 1;
  for (const unsigned char zero : zeros) {
    int pos = lim - gf.ToAlphaPow(zero);
    out[pos] = !out[pos];
  }

  return out;
}

int main(int argc, char** argv) {
  GF16 gf;

  std::vector<bool> in = {0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1};
  in[13] = !in[13];
  in[5] = !in[5];

  auto result = DecodeBCH(gf, in, 1, 7);
  if (absl::holds_alternative<std::string>(result)) {
    std::cerr << absl::get<std::string>(result) << "\n";
    return 1;
  }

  std::cout << absl::get<std::vector<bool>>(result) << "\n";

  return 0;
}
