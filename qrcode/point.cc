#include "qrcode/point.h"

#include "absl/strings/str_cat.h"

std::ostream& operator<<(std::ostream& stream, const Point& p) {
  return stream << absl::StrCat("(", p.x, ",", p.y, ")");
}
