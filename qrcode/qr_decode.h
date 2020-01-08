#ifndef _QRCODE_QR_DECODE_H_
#define _QRCODE_QR_DECODE_H_ 1

#include <memory>

#include "absl/types/variant.h"

#include "qrcode/qr_array.h"
#include "qrcode/qr_attributes.h"
#include "qrcode/qr_types.h"

struct QRCode {
  std::unique_ptr<QRAttributes> attributes;
  std::unique_ptr<QRCodeArray> unmasked_array;
  std::vector<CodewordBlock> codewords;
};

absl::variant<std::unique_ptr<QRCode>, std::string> Decode(
    std::unique_ptr<QRCodeArray> array);

#endif  // _QRCODE_QR_DECODE_H_
