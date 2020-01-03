#include "qrcode/qr_error_characteristics_types.h"

#include <iostream>

std::ostream& operator<<(std::ostream& str, const QRErrorCorrection level) {
  switch (level) {
    case QRECC_L:
      return str << "QRECC_L";
    case QRECC_M:
      return str << "QRECC_M";
    case QRECC_Q:
      return str << "QRECC_Q";
    case QRECC_H:
      return str << "QRECC_H";
  }
}
