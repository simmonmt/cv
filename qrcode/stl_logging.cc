#include "qrcode/stl_logging.h"

std::ostream& operator<<(std::ostream& str, const std::vector<bool>& vec) {
  for (int i = vec.size() - 1; i >= 0; --i) {
    str << int(vec[i]);
  }
  return str;
}
