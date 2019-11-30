#ifndef _QRCODE_QR_EXTRACT_H_
#define _QRCODE_QR_EXTRACT_H_ 1

#include <memory>
#include <vector>

#include "absl/types/variant.h"

#include "qrcode/qr_normalize.h"
#include "qrcode/qr_types.h"

absl::variant<std::unique_ptr<QRCodeArray>, std::string> ExtractCode(
    const QRImage& qr_image);

#endif  // _QRCODE_QR_EXTRACT_H_
