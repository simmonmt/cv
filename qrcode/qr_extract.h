#ifndef _QRCODE_QR_EXTRACT_H_
#define _QRCODE_QR_EXTRACT_H_ 1

#include "absl/types/variant.h"

#include "qrcode/qr_normalize.h"

absl::variant<std::string> ExtractCode(const QRImage& qr_image);

#endif  // _QRCODE_QR_EXTRACT_H_
