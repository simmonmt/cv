#include "qrcode/qr_error.h"

absl::variant<QRErrorCharacteristics::ECC, std::string> GetErrorCharacteristics(
    int version, QRErrorCorrection level) {
  return "unimplemented";
}
