#include "qrcode/qr_format.h"

#include "qrcode/bch_new.h"
#include "qrcode/gf.h"

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

absl::variant<QRFormat, std::string> DecodeFormat(const QRCodeArray& array) {
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

  // Format uses BCH(15,5), with what seem to be c=1 and d=7. The spec
  // doesn't give us values for c or d.
  //
  // https://en.m.wikipedia.org/wiki/BCH_code#Calculate_the_syndromes
  // says there are syndromes c to c+d-2. The spec's worked example
  // (only found in the 2000 version of the spec, section C.2) says to
  // calculate syndromes S1..S5, while the Wikipedia example
  // (https://en.m.wikipedia.org/wiki/BCH_code#Decoding_of_binary_code_without_unreadable_characters)
  // uses S1..S6. Either way starting with S1 implies that c=1.
  //
  // The Wikipedia example asserts that d=7, which is borne out by the
  // number of syndromes generated (S1..S6). The spec only shows
  // S1..S5, which is consistent with d=6.
  const int c = 1, d = 7;

  auto result1 = DecodeBCHNew(
      gf16, XORFormat(FindOneFormatCopy(array, kFormatCopy1)), c, d);
  auto result2 = DecodeBCHNew(
      gf16, XORFormat(FindOneFormatCopy(array, kFormatCopy2)), c, d);
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
  format.erase(format.begin(), format.end() - 5);

  QRFormat decoded;
  decoded.ecc_level = DecodeErrorCorrection((format[4] << 1) | format[3]);
  decoded.mask_pattern = (format[2] << 2) | (format[1] << 1) | format[0];
  return decoded;
}
