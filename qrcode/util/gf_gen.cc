// A utility that generates the alpha power values in GF(2^4) and
// GF(2^5) as well as the implementation of multiplication for that
// field.

#include <iostream>
#include <vector>

#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"
#include "absl/types/optional.h"

namespace {

struct Term {
  Term(char l, char r, int y) : l(l), r(r), y(y) {}

  char l, r;
  int y;
};

std::ostream& operator<<(std::ostream& s, const Term& t) {
  return s << t.l << t.r << "A^" << t.y;
}

std::string PowerString(const std::string& base, int pow) {
  if (pow == 0) {
    return "1";
  } else if (pow == 1) {
    return base;
  } else {
    return absl::StrFormat("%s^%d", base, pow);
  }
}

std::string PolyToString(unsigned char p) {
  std::string out;

  bool first = true;
  for (int i = 0; i < 8; i++) {
    if (p & 1) {
      if (first) {
        first = false;
      } else {
        out = "+ " + out;
      }
      out = PowerString("x", i) + out;
    }
    p >>= 1;
  }

  return out;
}

std::string ToBin(unsigned char v, int lim) {
  std::string out;
  for (int i = 0; i < lim; ++i) {
    out = ((v & 1) ? "1" : "0") + out;
    v >>= 1;
  }
  return out;
}

absl::optional<std::vector<unsigned char>> GenGFBits(int m, unsigned char p) {
  unsigned char val = 1;
  std::vector<unsigned char> bits;
  int i;
  for (i = 0; i < 100 && (i == 0 || val != 1); ++i) {
    bits.push_back(val);
    val <<= 1;
    if (val & (1 << m)) {
      val ^= p;
    }
  }

  if (i == 100) {
    return absl::nullopt;  // the loop didn't terminate
  }

  return bits;
}

void GenMultTerms(int m, const char* left, const char* right,
                  const std::vector<unsigned char>& bits) {
  std::cout << absl::StrFormat("=== GF(2^%d) multiplication (%*.*s * %*.*s)\n",
                               m, m, m, left, m, m, right);

  std::vector<Term> terms;
  for (int i = 0; i < m; ++i) {
    for (int j = 0; j < m; ++j) {
      terms.push_back(Term(left[i], right[j], i + j));
    }
  }

  std::cout << "terms: ";
  for (const Term& t : terms) {
    std::cout << t << " ";
  }
  std::cout << "\n";

  std::vector<Term> simp;
  for (const Term& t : terms) {
    unsigned char pat = bits[t.y];
    for (int i = 0; i < m; ++i) {
      if (pat & 1) {
        simp.push_back(Term(t.l, t.r, i));
      }
      pat >>= 1;
    }
  }

  std::cout << "simplified: ";
  for (const Term& t : simp) {
    std::cout << t << " ";
  }
  std::cout << "\n";

  std::cout << "grouped:\n";
  for (int pow = 0; pow < m; ++pow) {
    std::cout << "(";
    bool first = true;
    for (const Term& t : simp) {
      if (t.y == pow) {
        if (first) {
          first = false;
        } else {
          std::cout << " + ";
        }
        std::cout << t.l << t.r;
      }
    }
    std::cout << ") * " << PowerString("alpha", pow) << "\n";
  }

  std::cout << "calculation of output bits:\n";
  for (int pow = 0; pow < m; ++pow) {
    std::cout << "out_" << left[pow] << " = ";
    bool first = true;
    for (const Term& t : simp) {
      if (t.y == pow) {
        if (first) {
          first = false;
        } else {
          std::cout << " ^ ";
        }
        std::cout << "(" << t.l << "&&" << t.r << ")";
      }
    }
    std::cout << ";\n";
  }
}

void PrintGF(int m, unsigned char p) {
  std::cout << "=== GF(2^" << m << ") poly " << PolyToString(p) << "\n";
  auto maybe_bits = GenGFBits(m, p);
  if (!maybe_bits.has_value()) {
    std::cout << "didn't terminate\n";
    return;
  }

  const std::vector<unsigned char> bits = *maybe_bits;
  for (int i = 0; i < bits.size(); ++i) {
    std::cout << i << ": " << ToBin(bits[i], 5) << "\n";
  }

  for (int i = 0; i < bits.size(); ++i) {
    std::cout << int(bits[i]) << ", ";
  }
  std::cout << "\n";

  static constexpr char kVars[] = "abcdefghijklmnopqrstuvwxyz";
  const char* left = kVars;
  const char* right = kVars + m;

  GenMultTerms(m, left, right, bits);
}

}  // namespace

int main(int argc, char** argv) {
  PrintGF(4, 0x13 /* x^4+x+1 */);
  PrintGF(5, 0x25 /* x^5+x^2+1 */);
  return 0;
}
