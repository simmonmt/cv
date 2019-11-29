#include "qrcode/qr_decode.h"

absl::variant<std::unique_ptr<QRCode>, std::string> Decode(
    const QRCodeArray& array) {
  return "unimplemented";
}
