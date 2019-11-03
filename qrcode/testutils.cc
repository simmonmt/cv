#include "qrcode/testutils.h"

std::vector<unsigned char> MakeRun(std::vector<int> lens) {
  std::vector<unsigned char> out;
  for (int i = 0; i < lens.size(); ++i) {
    const int len = lens[i];
    std::vector<unsigned char> run(len, (i % 2 == 0) ? 255 : 0);
    out.insert(out.end(), run.begin(), run.end());
  }
  return out;
}
