#include "qrcode/bch_new.h"

#include <bitset>
#include <cmath>
#include <iostream>
#include <sstream>

#include "absl/memory/memory.h"
#include "absl/types/variant.h"
#include "qrcode/stl_logging.h"

#include "qrcode/gf.h"
#include "qrcode/gf_mat.h"

namespace {

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

std::string GFVecToString(const GF& gf, const std::vector<unsigned char>& vec) {
  std::stringstream ss;
  for (const auto& elem : vec) {
    if (ss.tellp() != 0) {
      ss << " ";
    }
    ss << std::bitset<8>(elem);
  }
  return ss.str();
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
// It returns a set of coefficients lambda_{1..v} that form the following
// polynomial:
//
//   lambda(x) = lambda_1 * x + lambda_2 * x^2 + ... + lambda_v * x^v
//
// An empty vector is returned if an error occurs -- if the polynomial cannot be
// derived.
std::vector<unsigned char> PGZ(const GF& gf, int c, int t,
                               const std::vector<unsigned char>& syndromes) {
  int v = t;

  for (; v > 0;) {
    // std::cout << "PGZ attempt with v " << v << "\n";

    // Steps 1 and 2: Make matrices S_{v,v} and C_{v,1}
    GFSqMat a(gf, v);
    std::vector<unsigned char> cvec(v);
    for (int y = 0; y < v; ++y) {
      for (int x = 0; x < v; ++x) {
        a.Set(x, y, syndromes[c + x + y]);
      }
      cvec[y] = syndromes[c + v + y];
    }

    // std::cout << "starting a:\n";
    // a.Print();
    // std::cout << "starting c: " << GFVecToString(gf, cvec) << "\n";

    // There's no code for steps 3 or 4 since they're just defining terms we'll
    // use later, namely that there exists lambda_{v,1} such that
    //
    //   S_{v,v} * lambda_{v,1} = C{v,1}
    //
    // The whole point of this exercise is to find lambda_{v,1} for some v.
    //
    // Note that because we're in a GF(2^x) field addition is the same as
    // subtraction, so PGZ's -C_{v,1} = C_{v,1}.

    unsigned char det = a.Determinant();

    // std::cout << "determinant is " << std::bitset<8>(det) << "\n";

    // Step 6: If the determinant is zero, try again with v--.
    if (det == 0) {
      v--;
      continue;
    }

    // Step 5: The determinant is non-zero, which means we can invert S_{v,v}
    // which in turn means we can solve the above equation for
    // lambda_{v,1}.
    std::unique_ptr<GFSqMat> inv_a = a.Inverse();

    // Step 5 (cont): Calculate S^{-1} * C, which gives us the error
    // locator polynomial coefficients lambda_1, lambda_2, ..., lambda_v,
    // forming this polynomial:
    //
    // lambda(x) = 1 + lambda_1 * x + lambda_2 * x^2 + ... + lambda_v * x^v
    //
    // The vector `lambda` contains these coefficients in order, starting with a
    // lambda_0=1.
    std::vector<unsigned char> lambda(inv_a->h());
    for (int i = 0; i < inv_a->h(); ++i) {
      lambda[v - i - 1] = DotProduct(gf, inv_a->Row(i), cvec);
    }

    // std::cout << "found lambda " << GFVecToString(gf, lambda) << "\n";

    // Step 7: We're done.
    return lambda;
  }

  return {};
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

absl::variant<std::vector<bool>, std::string> DecodeBCHNew(
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

  // for (int i = s_lo; i <= s_hi; ++i) {
  //   std::cout << "syndrome " << i << ": " << std::bitset<8>(syndromes[i])
  //             << "\n";
  // }

  if (all_zero) {
    return bits;
  }

  int t = (d - 1) / 2;

  // std::cout << "t=" << t << "\n";

  std::vector<unsigned char> lambda = PGZ(gf, c, t, syndromes);
  if (lambda.empty()) {
    return "no lambda found";
  }
  lambda.insert(lambda.begin(), 1);

  std::cout << "got lambda " << GFVecToString(gf, lambda) << "\n";

  std::vector<unsigned char> zeros = FindZeros(gf, lambda);
  if (zeros.empty()) {
    return "no zeros found";
  }

  std::cout << "got zeros vec " << GFVecToString(gf, zeros) << "\n";

  std::vector<bool> out = bits;
  int lim = std::pow(2, gf.m()) - 1;
  for (const unsigned char zero : zeros) {
    int pos = (lim - gf.ToAlphaPow(zero)) % lim;
    std::cout << "zero " << std::bitset<4>(zero) << " = alpha^"
              << gf.ToAlphaPow(zero) << "; flipping " << pos << "\n";
    out[pos] = !out[pos];
  }

  return out;
}
