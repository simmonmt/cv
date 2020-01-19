#include "qrcode/bch.h"

#include <iostream>

namespace {

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
  const unsigned char s1 = R(gf, bits, 1);
  const unsigned char s3 = R(gf, bits, 3);
  const unsigned char s5 = R(gf, bits, 5);

  if (s1 == 0 && s3 == 0 && s5 == 0) {
    return bits;
  }

  const unsigned char s2 = gf.Pow(s1, 2);
  const unsigned char s4 = gf.Pow(s2, 2);

  // Solve Eq1: S1 + d1 = 0
  const unsigned char d1 = s1;

  // Eq2: S3 + S2*d1 + S1*d2 + d3 = 0
  const unsigned char s3_plus_s2d1 = gf.Add({s3, gf.Mult(s2, d1)});
  auto eq2 = [&](unsigned char d2, unsigned char d3) {
    unsigned char res = s3_plus_s2d1;
    res = gf.Add({res, gf.Mult(s1, d2)});
    res = gf.Add({res, d3});
    return res;
  };

  // Eq3: S5 + S4*d1 + S3*d2 + S2*d3 = 0
  const unsigned char s5_plus_s4d1 = gf.Add({s5, gf.Mult(s4, d1)});
  auto eq3 = [&](unsigned char d2, unsigned char d3) {
    unsigned char res = s5_plus_s4d1;
    res = gf.Add({res, gf.Mult(s3, d2)});
    res = gf.Add({res, gf.Mult(s2, d3)});
    return res;
  };

  bool found = false;
  unsigned char d2, d3;
  const std::vector<unsigned char> gf_elements = gf.Elements();
  for (int i = 0; !found && i < gf_elements.size(); ++i) {
    for (int j = 0; !found && j < gf_elements.size(); ++j) {
      d2 = gf_elements[i];
      d3 = gf_elements[j];
      if (eq2(d2, d3) == 0 && eq3(d2, d3) == 0) {
        found = true;
      }
    }
  }

  if (!found) {
    return "failed to find d2/d3";
  }

  if (gf.Add({s1, d1}) != 0) {
    return "internal error: eq1 doesn't verify";
  }
  if (gf.Add({s3, gf.Mult(s2, d1), gf.Mult(s1, d2), d3}) != 0) {
    return "internal error: eq2 doesn't verify";
  }
  if (gf.Add({s5, gf.Mult(s4, d1), gf.Mult(s3, d2), gf.Mult(s2, d3)}) != 0) {
    return "internal error: eq3 doesn't verify";
  }

  std::vector<int> errors;
  const std::vector<unsigned char>& powers_of_alpha = gf.PowersOfAlpha();
  for (int i = 0; i < powers_of_alpha.size(); ++i) {
    // Look for x^3 + d1*x^2 + d2*x + d3 == 0
    unsigned char x = powers_of_alpha[i];
    unsigned char res = gf.Add({gf.Pow(x, 3),               //
                                gf.Mult(d1, gf.Pow(x, 2)),  //
                                gf.Mult(d2, x),             //
                                d3});

    if (res == 0) {
      errors.push_back(i);
    }
  }

  std::vector<bool> out = bits;
  for (int i : errors) {
    out[i] = !out[i];
  }
  return out;
}
