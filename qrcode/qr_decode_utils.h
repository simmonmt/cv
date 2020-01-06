#ifndef _QRCODE_QR_DECODE_UTILS_H_
#define _QRCODE_QR_DECODE_UTILS_H_ 1

#include "absl/types/variant.h"

#include "qrcode/qr_array.h"
#include "qrcode/qr_attributes.h"

struct QRFormat {
  QRErrorCorrection ecc_level;
  unsigned mask_pattern;
};

absl::variant<QRFormat, std::string> DecodeFormat(const QRCodeArray& array);

void UnmaskArray(const QRAttributes& attributes, QRCodeArray* array,
                 unsigned char mask_pattern);

#endif  // _QRCODE_QR_DECODE_UTILS_H_
