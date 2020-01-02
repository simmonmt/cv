#include "qrcode/qr_error.h"

#include <iostream>

#include "absl/strings/str_format.h"

namespace {

constexpr QRErrorCharacteristics kCharacteristics[] = {};

}  // namespace

absl::variant<QRErrorCharacteristics::ECC, std::string> GetErrorCharacteristics(
    int version, QRErrorCorrection level) {
  for (const auto& characteristics : kCharacteristics) {
    if (characteristics.version == version &&
        characteristics.ecc[level].num_codewords != -1) {
      return characteristics.ecc[level];
    }
  }

  std::stringstream str;
  str << "no error characteristics for version " << version << " level "
      << level;
  return str.str();
}

std::ostream& operator<<(std::ostream& str, const QRErrorCorrection level) {
  switch (level) {
    case QRECC_L:
      return str << "QRECC_L";
    case QRECC_M:
      return str << "QRECC_M";
    case QRECC_Q:
      return str << "QRECC_Q";
    case QRECC_H:
      return str << "QRECC_H";
  }
}
