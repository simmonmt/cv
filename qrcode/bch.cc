#include "qrcode/bch.h"

absl::variant<std::vector<bool>, std::string> DecodeBCH(
    const GF& gf, const std::vector<bool>& bits) {
  return "unimplemented";
}
