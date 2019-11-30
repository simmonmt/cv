#include "qrcode/qr_decode.h"

#include <memory>

#include "absl/memory/memory.h"

absl::variant<std::unique_ptr<QRCode>, std::string> Decode(
    const QRCodeArray& array) {
  auto qrcode = absl::make_unique<QRCode>();
  qrcode->width = array.width();
  qrcode->height = array.height();

  // Version decode
  //   ((D/X)-10)/4 from the ref decode algorithm, with X=1, D
  //   measured from positioning point X centers (i.e. left+3).
  qrcode->version = ((array.width() - 6) - 10) / 4;
  if (qrcode->version > 6) {
    // TODO: implement
    return "large version decoding unimplemented";
  }

  return std::move(qrcode);
}
