#ifndef _QRCODE_QR_ERROR_H_
#define _QRCODE_QR_ERROR_H_ 1

#include <string>

#include "absl/types/variant.h"

enum QRErrorCorrection {
  QRECC_L,
  QRECC_M,
  QRECC_Q,
  QRECC_H,
};

struct QRErrorCharacteristics {
  int version;

  struct ECC {
    int num_codewords;
    int num_data_codewords;
  };

  ECC ecc[4];  // indexed by QRErrorCorrection
};

absl::variant<QRErrorCharacteristics::ECC, std::string> GetErrorCharacteristics(
    int version, QRErrorCorrection level);

#endif  // _QRCODE_QR_ERROR_H_
