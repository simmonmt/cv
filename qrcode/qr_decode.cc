#include "qrcode/qr_decode.h"

#include <iostream>
#include <memory>

#include "absl/memory/memory.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"

#include "qrcode/qr_attributes.h"
#include "qrcode/qr_decode_utils.h"
#include "qrcode/qr_format.h"

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
  const QRFormat format = std::move(absl::get<QRFormat>(format_result));

  auto attributes_result = QRAttributes::New(version, format.ecc_level);
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
  UnmaskArray(*attributes, array.get(), format.mask_pattern);

  auto qrcode = absl::make_unique<QRCode>();
  qrcode->attributes = std::move(attributes);
  qrcode->unmasked_array = std::move(array);
  return std::move(qrcode);
}
