#ifndef _QRCODE_QR_ERROR_CHARACTERISTICS_H_
#define _QRCODE_QR_ERROR_CHARACTERISTICS_H_ 1

#include <string>
#include <vector>

#include "absl/types/variant.h"
#include "qrcode/qr_error_characteristics_types.h"

absl::variant<QRErrorLevelCharacteristics, std::string> GetErrorCharacteristics(
    int version, QRErrorCorrection level);

#endif  // _QRCODE_QR_ERROR_CHARACTERISTICS_H_
