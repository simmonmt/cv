#include "qrcode/qr_error_characteristics.h"

#include <sstream>

#include "qrcode/qr_error_characteristics_types.h"

// Provides the kQRErrorCharacterists array
#include "qrcode/qr_error_characteristics_data.h"

absl::variant<QRErrorLevelCharacteristics, std::string> GetErrorCharacteristics(
    int version, QRErrorCorrection level) {
  for (const ErrorCharacteristicsDesc& desc : kQRErrorCharacteristics) {
    if (version != desc.version) {
      continue;
    }

    return desc.levels[static_cast<int>(level)];
  }

  std::stringstream str;
  str << "no error characteristics for version " << version << " level "
      << level;
  return str.str();
}
