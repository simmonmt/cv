#include "qrcode/qr_attributes.h"

#include "absl/memory/memory.h"
#include "absl/strings/str_cat.h"

namespace {

constexpr int kModulesPerSide[] = {-1, 21, 25, 29, 33, 37, 41};

constexpr int kAlignmentCenters[][6] = {
    {},       // v0
    {},       // v1
    {6, 18},  // v2
    {6, 22},  // v3
    {6, 26},  // v4
    {6, 30},  // v5
    {6, 34},  // v6
};

void WriteBlock(const Point& top_left, const Point& bottom_right,
                int modules_per_side, QRAttributes::ModuleType module_type,
                std::vector<QRAttributes::ModuleType>* type_map) {
  for (int y = top_left.y; y <= bottom_right.y; y++) {
    for (int x = top_left.x; x <= bottom_right.x; x++) {
      int off = y * modules_per_side + x;
      (*type_map)[off] = module_type;
    }
  }
}

void WriteAlignmentBlock(const Point& center, int modules_per_side,
                         std::vector<QRAttributes::ModuleType>* type_map) {
  WriteBlock(Point(center.x - 2, center.y - 2),
             Point(center.x + 2, center.y + 2), modules_per_side,
             QRAttributes::TYPE_ALIGNMENT_PATTERN, type_map);
}

absl::variant<std::unique_ptr<std::vector<QRAttributes::ModuleType>>,
              std::string>
MakeTypeMap(const int version, const int modules_per_side) {
  auto type_map = absl::make_unique<std::vector<QRAttributes::ModuleType>>(
      modules_per_side * modules_per_side, QRAttributes::TYPE_DATA);

  // Positioning blocks
  WriteBlock(Point(0, 0), Point(7, 7), modules_per_side,
             QRAttributes::TYPE_POSITION_DETECTION_PATTERN, type_map.get());
  WriteBlock(Point(0, modules_per_side - 8), Point(7, modules_per_side - 1),
             modules_per_side, QRAttributes::TYPE_POSITION_DETECTION_PATTERN,
             type_map.get());
  WriteBlock(Point(modules_per_side - 8, 0), Point(modules_per_side - 1, 7),
             modules_per_side, QRAttributes::TYPE_POSITION_DETECTION_PATTERN,
             type_map.get());

  // Format
  WriteBlock(Point(8, 0), Point(8, 8), modules_per_side,
             QRAttributes::TYPE_FORMAT_INFORMATION, type_map.get());
  WriteBlock(Point(0, 8), Point(7, 8), modules_per_side,
             QRAttributes::TYPE_FORMAT_INFORMATION, type_map.get());
  WriteBlock(Point(8, modules_per_side - 8), Point(8, modules_per_side - 1),
             modules_per_side, QRAttributes::TYPE_FORMAT_INFORMATION,
             type_map.get());
  WriteBlock(Point(modules_per_side - 8, 8), Point(modules_per_side - 1, 8),
             modules_per_side, QRAttributes::TYPE_FORMAT_INFORMATION,
             type_map.get());

  // Version
  if (version >= 7) {
    return "versions containing version block not supported";
  }

  // Timing
  WriteBlock(Point(8, 6), Point(modules_per_side - 9, 6), modules_per_side,
             QRAttributes::TYPE_TIMING_PATTERN, type_map.get());
  WriteBlock(Point(6, 8), Point(6, modules_per_side - 9), modules_per_side,
             QRAttributes::TYPE_TIMING_PATTERN, type_map.get());

  // Alignment
  //
  // Any combination of the values that aren't currently occupied by
  // other things (mainly position detection patterns).
  for (const int& c1 : kAlignmentCenters[version]) {
    if (c1 == 0) {
      break;
    }

    for (const int& c2 : kAlignmentCenters[version]) {
      if (c2 == 0) {
        break;
      }

      Point center(c1, c2);
      int off = modules_per_side * center.y + center.x;
      if ((*type_map)[off] == QRAttributes::TYPE_DATA) {
        WriteAlignmentBlock(center, modules_per_side, type_map.get());
      }
    }
  }

  return type_map;
}

}  // namespace

QRAttributes::QRAttributes(int version, QRErrorCorrection ecc_level,
                           int modules_per_side,
                           std::unique_ptr<std::vector<ModuleType>> type_map,
                           QRErrorLevelCharacteristics error_characteristics)
    : version_(version),
      ecc_level_(ecc_level),
      modules_per_side_(modules_per_side),
      type_map_(std::move(type_map)),
      error_characteristics_(error_characteristics) {}

absl::variant<std::unique_ptr<QRAttributes>, std::string> QRAttributes::New(
    int version, QRErrorCorrection level) {
  if (version == 0 || version >= ABSL_ARRAYSIZE(kModulesPerSide)) {
    return absl::StrCat("unsupported/unknown version ", version);
  }
  const int modules_per_side = kModulesPerSide[version];

  auto type_map_result = MakeTypeMap(version, modules_per_side);
  if (absl::holds_alternative<std::string>(type_map_result)) {
    return absl::StrCat("failed to make type map: ",
                        absl::get<std::string>(type_map_result));
  }
  auto type_map = std::move(
      absl::get<std::unique_ptr<std::vector<ModuleType>>>(type_map_result));

  auto error_result = GetErrorCharacteristics(version, level);
  if (absl::holds_alternative<std::string>(error_result)) {
    return absl::StrCat("failed to get error characteristics: ",
                        absl::get<std::string>(error_result));
  }

  QRErrorLevelCharacteristics error_characteristics =
      std::move(absl::get<QRErrorLevelCharacteristics>(error_result));

  return absl::WrapUnique(new QRAttributes(version, level, modules_per_side,
                                           std::move(type_map),
                                           error_characteristics));
}

QRAttributes::ModuleType QRAttributes::GetModuleType(Point p) const {
  if (p.x < 0 || p.y < 0 || p.x >= modules_per_side_ ||
      p.y >= modules_per_side_) {
    return TYPE_UNKNOWN;
  }

  int off = p.y * modules_per_side_ + p.x;
  return (*type_map_)[off];
}

char QRAttributes::ModuleTypeToChar(ModuleType t) {
  switch (t) {
    case TYPE_UNKNOWN:
      return '?';
    case TYPE_POSITION_DETECTION_PATTERN:
      return 'P';
    case TYPE_FORMAT_INFORMATION:
      return 'F';
    case TYPE_VERSION_INFORMATION:
      return 'V';
    case TYPE_TIMING_PATTERN:
      return 'T';
    case TYPE_ALIGNMENT_PATTERN:
      return 'A';
    case TYPE_DATA:
      return '.';
  }
}
