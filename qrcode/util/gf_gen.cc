#include <iostream>
#include <vector>

namespace {

struct Term {
  Term(char l, char r, int y) : l(l), r(r), y(y) {}

  char l, r;
  int y;
};

std::ostream& operator<<(std::ostream& s, const Term& t) {
  return s << t.l << t.r << "A^" << t.y;
}

void GenMultTerms(int m, const char* left, const char* right, const int* bits) {
  std::cout << "=== m=" << m << " mult\n";

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
    char pat = bits[t.y];
    for (int i = 0; i < m; ++i) {
      if (pat & 1) {
        simp.push_back(Term(t.l, t.r, i));
      }
      pat >>= 1;
    }
  }

  std::cout << "simp: ";
  for (const Term& t : simp) {
    std::cout << t << " ";
  }
  std::cout << "\n";

  for (int pow = 0; pow < m; ++pow) {
    std::cout << pow << " ";
    for (const Term& t : simp) {
      if (t.y == pow) {
        std::cout << t.l << t.r << " ";
      }
    }
    std::cout << "\n";
  }

  for (int pow = 0; pow < m; ++pow) {
    std::cout << pow << " ";
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
    std::cout << "\n";
  }
}

}  // namespace

int main(int argc, char** argv) {
  std::cout << "gf_gen\n";

  // Bits for GF(2^4)
  static const int kGF24Bits[] = {1, 2,  4, 8,  3,  6,  12, 11,
                                  5, 10, 7, 14, 15, 13, 9};
  GenMultTerms(4, "abcd", "efgh", kGF24Bits);

  // Bits for GF(2^5)
  // alpha^5 is a guess based on the 2^4 pattern.
  static const int kGF25Bits[] = {1, 2, 4, 8, 16, 3, 6, 12, 24};
  GenMultTerms(5, "abcde", "fghij", kGF25Bits);

  return 0;
}
