#include "qrcode/qr_decode_utils.h"

#include "qrcode/array_walker.h"
#include "qrcode/bch.h"
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
  format.erase(format.begin(), format.end() - 5);

  QRFormat decoded;
  decoded.ecc_level = DecodeErrorCorrection((format[4] << 1) | format[3]);
  decoded.mask_pattern = (format[2] << 2) | (format[1] << 1) | format[0];
  return decoded;
}

void UnmaskArray(const QRAttributes& attributes, QRCodeArray* array,
                 unsigned char mask_pattern) {
  typedef std::function<bool(const Point&)> Unmasker;
  static Unmasker unmaskers[8] = {
      [](const Point& p) { return (p.x + p.y) % 2 == 0; },          // 000
      [](const Point& p) { return p.x % 2 == 0; },                  // 001
      [](const Point& p) { return p.y % 3 == 0; },                  // 010
      [](const Point& p) { return (p.x + p.y) % 3 == 0; },          // 011
      [](const Point& p) { return (p.x / 2 + p.y / 3) % 2 == 0; },  // 100
      [](const Point& p) {
        return (p.x * p.y) % 2 + (p.x * p.y) % 3 == 0;
      },  // 101
      [](const Point& p) {
        return ((p.x * p.y) % 2 + (p.x * p.y) % 3) % 2 == 0;
      },  // 110
      [](const Point& p) {
        return ((p.x * p.y) % 3 + (p.x + p.y) % 2) % 2 == 0;
      },  // 111
  };

  Unmasker* unmasker = &unmaskers[mask_pattern];

  for (int y = 0; y < attributes.modules_per_side(); ++y) {
    for (int x = 0; x < attributes.modules_per_side(); ++x) {
      Point p(x, y);
      if (attributes.GetModuleType(p) != QRAttributes::TYPE_DATA) {
        continue;
      }

      array->Set(p, array->Get(p) ^ (*unmasker)(p));
    }
  }
}

std::vector<unsigned char> FindDataBlocks(const QRAttributes& attributes,
                                          const QRCodeArray& array) {
  ArrayWalker walker(attributes);
  std::vector<unsigned char> out;

  int num_bits = 0;
  unsigned char cur_val = 0;
  for (;;) {
    absl::optional<Point> p = walker.Next();
    if (!p.has_value()) {
      break;
    }

    cur_val = (cur_val << 1) | array.Get(*p);
    num_bits++;

    if (num_bits == 8) {
      out.push_back(cur_val);
      cur_val = 0;
      num_bits = 0;
    }
  }

  if (num_bits > 0) {
    cur_val <<= 8 - num_bits;
    out.push_back(cur_val);
  }

  return out;
}
