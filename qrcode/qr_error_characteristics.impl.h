#ifndef _QRCODE_QR_ERROR_CHARACTERISTICS_IMPL_H_
#define _QRCODE_QR_ERROR_CHARACTERISTICS_IMPL_H_ 1

#include <vector>

#include "qrcode/qr_error_characteristics_types.h"

struct ErrorCharacteristicsDesc {
  int version;
  QRErrorLevelCharacteristics levels[4];
};

#endif  // _QRCODE_QR_ERROR_CHARACTERISTICS_IMPL_H_
