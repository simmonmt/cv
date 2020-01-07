#ifndef _QRCODE_QR_FORMAT_H_
#define _QRCODE_QR_FORMAT_H_ 1

#include <string>

#include "absl/types/variant.h"

#include "qrcode/qr_array.h"
#include "qrcode/qr_error_characteristics_types.h"

struct QRFormat {
  QRErrorCorrection ecc_level;
  unsigned mask_pattern;
};

absl::variant<QRFormat, std::string> DecodeFormat(const QRCodeArray& array);

#endif  // _QRCODE_QR_FORMAT_H_
