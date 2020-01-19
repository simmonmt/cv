// Generates the multiplicative inverses for a given 2^n Galois field. This
// can't be part of gf_gen because it depends on the creation of a GF subclass
// with a Mult implemented as described by the gf_gen output.

#include <bitset>
#include <cmath>
#include <iostream>

#include "qrcode/gf.h"

int main(int argc, char** argv) {
  GF256 gf;

  const std::vector<unsigned char>& powers_of_alpha = gf.PowersOfAlpha();
  std::vector<unsigned char> inverses(std::pow(2, gf.m()));

  bool errors = false;
  for (const unsigned char value : powers_of_alpha) {
    bool found = false;
    for (int j = 0; !found && j < powers_of_alpha.size(); ++j) {
      unsigned char inv = powers_of_alpha[j];
      if (gf.Mult(value, inv) != 1) {
        continue;
      }

      if (inverses[value] != 0) {
        // This should never happen as it means there are duplicates in
        // PowersOfAlpha.
        std::cerr << "duplicate for " << std::bitset<8>(value) << "\n";
        errors = true;
        break;
      }

      inverses[value] = inv;
      found = true;
    }

    if (!found) {
      std::cerr << "no inverse for " << std::bitset<8>(value) << "\n";
      errors = true;
    }
  }

  if (errors) {
    std::cerr << "errors found\n";
    return 1;
  }

  for (int i = 0; i < inverses.size(); i++) {
    if (i > 0) {
      std::cout << ",";
    }
    std::cout << int(inverses[i]);
  }
  std::cout << "\n";
  return 0;
}
