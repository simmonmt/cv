#ifndef _QRCODE_QR_DECODE_H_
#define _QRCODE_QR_DECODE_H_ 1

#include <memory>

#include "absl/types/variant.h"

#include "qrcode/qr_extract.h"

struct QRCode {
  int version;
  int height;
  int width;
};

absl::variant<std::unique_ptr<QRCode>, std::string> Decode(
    const QRCodeArray& array);

#endif  // _QRCODE_QR_DECODE_H_
