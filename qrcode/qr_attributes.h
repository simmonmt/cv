#ifndef _QRCODE_QR_ATTRIBUTES_H_
#define _QRCODE_QR_ATTRIBUTES_H_ 1

#include <memory>
#include <string>

#include "qrcode/point.h"
#include "qrcode/qr_error_characteristics.h"

class QRAttributes {
 public:
  ~QRAttributes() = default;

  static absl::variant<std::unique_ptr<QRAttributes>, std::string> New(
      int version, QRErrorCorrection ecc_level);

  int version() const { return version_; }
  QRErrorCorrection ecc_level() const { return ecc_level_; }
  int modules_per_side() const { return modules_per_side_; }

  enum ModuleType {
    TYPE_UNKNOWN,
    TYPE_POSITION_DETECTION_PATTERN,  // Includes separators
    TYPE_FORMAT_INFORMATION,
    TYPE_VERSION_INFORMATION,
    TYPE_TIMING_PATTERN,
    TYPE_ALIGNMENT_PATTERN,
    TYPE_DATA,
  };

  ModuleType GetModuleType(Point p) const;

  static char ModuleTypeToChar(ModuleType t);

  const QRErrorLevelCharacteristics& error_characteristics() const {
    return error_characteristics_;
  }

 private:
  QRAttributes(int version, QRErrorCorrection ecc_level, int modules_per_side,
               std::unique_ptr<std::vector<ModuleType>> type_map,
               QRErrorLevelCharacteristics error_characteristics);

  const int version_;
  const QRErrorCorrection ecc_level_;
  const int modules_per_side_;
  std::unique_ptr<std::vector<ModuleType>> type_map_;
  const QRErrorLevelCharacteristics error_characteristics_;
};

#endif  // _QRCODE_QR_ATTRIBUTES_H_
