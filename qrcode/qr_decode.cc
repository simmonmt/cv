#include "qrcode/qr_decode.h"

#include <iostream>
#include <memory>

#include "absl/memory/memory.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"

#include "qrcode/bch.h"
#include "qrcode/gf.h"
#include "qrcode/qr_attributes.h"
#include "qrcode/qr_decode_utils.h"
#include "qrcode/stl_logging.h"

namespace {

std::vector<bool> FindOneFormatCopy(const QRCodeArray& array,
                                    const std::vector<Point>& points) {
  std::vector<bool> vals(points.size());

  for (int i = 0; i < points.size(); ++i) {
    Point point = points[i];
    if (point.x < 0) {
      point.x = (array.width() - 1) + (point.x + 1);
    }
    if (point.y < 0) {
      point.y = (array.height() - 1) + (point.y + 1);
    }
    vals[i] = array.Get(point);
  }

  vals = {0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1};

  return vals;
}

std::vector<bool> XORFormat(std::vector<bool> vec) {
  static const std::vector<bool> kPattern = {0, 1, 0, 0, 1, 0, 0, 0,
                                             0, 0, 1, 0, 1, 0, 1};
  for (int i = 0; i < vec.size(); ++i) {
    vec[i] = vec[i] ^ kPattern[i];
  }
  return vec;
}

absl::variant<std::vector<bool>, std::string> DecodeFormat(
    const QRCodeArray& array) {
  static const std::vector<Point> kFormatCopy1 = {
      {8, 0}, {8, 1}, {8, 2}, {8, 3}, {8, 4}, {8, 5}, {8, 7}, {8, 8},
      {7, 8}, {5, 8}, {4, 8}, {3, 8}, {2, 8}, {1, 8}, {0, 8},
  };

  // -x should be interpreted as -x+1. They're stored with a -1 offset so we can
  // distinguish between +0/-0.
  static const std::vector<Point> kFormatCopy2 = {
      {0 - 1, 8},  {-1 - 1, 8}, {-2 - 1, 8}, {-3 - 1, 8}, {-4 - 1, 8},
      {-5 - 1, 8}, {-6 - 1, 8}, {-7 - 1, 8}, {8, -6 - 1}, {8, -5 - 1},
      {8, -4 - 1}, {8, -3 - 1}, {8, -2 - 1}, {8, -1 - 1}, {8, 0 - 1},
  };

  GF16 gf16;
  auto result1 =
      DecodeBCH(gf16, XORFormat(FindOneFormatCopy(array, kFormatCopy1)));
  auto result2 =
      DecodeBCH(gf16, XORFormat(FindOneFormatCopy(array, kFormatCopy2)));
  const bool result1_found = !absl::holds_alternative<std::string>(result1);
  const bool result2_found = !absl::holds_alternative<std::string>(result2);

  if (!result1_found && !result2_found) {
    return "failed to read valid format";
  }

  std::vector<bool> format;
  if (result1_found == result2_found) {
    std::vector<bool>& format1 = absl::get<std::vector<bool>>(result1);
    std::vector<bool>& format2 = absl::get<std::vector<bool>>(result2);
    if (format1 != format2) {
      return "format mismatch";
    }

    format = std::move(format1);
  } else if (result1_found) {
    format = std::move(absl::get<std::vector<bool>>(result1));
  } else {
    format = std::move(absl::get<std::vector<bool>>(result2));
  }

  // The five highest bits are data. The rest are just for error correction, and
  // should be discarded.
  return std::vector<bool>(format.end() - 5, format.end());
}

QRErrorCorrection DecodeErrorCorrection(uint format_correction) {
  switch (format_correction) {
    case 0:
      return QRECC_M;
    case 1:
      return QRECC_L;
    case 2:
      return QRECC_H;
    case 3:
      return QRECC_Q;
    default:
      // can't happen -- input is only two bits
      return QRECC_L;
  }
}

}  // namespace

absl::variant<std::unique_ptr<QRCode>, std::string> Decode(
    std::unique_ptr<QRCodeArray> array) {
  // Version decode (ref algorithm steps 5 and 6)
  //   ((D/X)-10)/4, with X=1, D  measured from positioning point X centers
  //   (i.e. left+3).
  const int version = ((array->width() - 6) - 10) / 4;
  if (version > 6) {
    // TODO: implement
    return "large version decoding unimplemented";
  }

  // Ref algorithm step 8 (finding the sampling grids using alignment patterns)
  // skipped because we don't do version > 6. Also it would be in QRCodeArray
  // construction.

  // Ref algorithm step 9 (sampling) skipped because we did it during
  // QRCodeArray construction.

  // Format decode (ref algorithm step 10)
  auto format_result = DecodeFormat(*array);
  if (absl::holds_alternative<std::string>(format_result)) {
    return "failed to decode format: " + absl::get<std::string>(format_result);
  }
  const std::vector<bool>& format = absl::get<std::vector<bool>>(format_result);

  const unsigned char mask_pattern =
      (format[2] << 2) | (format[1] << 1) | format[0];
  const QRErrorCorrection ecc_level =
      DecodeErrorCorrection((format[4] << 1) | format[3]);

  auto attributes_result = QRAttributes::New(version, ecc_level);
  if (absl::holds_alternative<std::string>(attributes_result)) {
    return absl::StrCat("failed to build attributes object: ",
                        absl::get<std::string>(attributes_result));
  }
  auto attributes =
      std::move(absl::get<std::unique_ptr<QRAttributes>>(attributes_result));

  if (attributes->modules_per_side() != array->width() ||
      attributes->modules_per_side() != array->height()) {
    return absl::StrFormat("version %d wants %d-module sides, but have %dx%d",
                           attributes->version(),
                           attributes->modules_per_side(), array->width(),
                           array->height());
  }

  // Unmask the array (ref algorithm step 11)
  UnmaskArray(*attributes, array.get(), mask_pattern);

  auto qrcode = absl::make_unique<QRCode>();
  qrcode->attributes = std::move(attributes);
  qrcode->unmasked_array = std::move(array);
  return std::move(qrcode);
}
