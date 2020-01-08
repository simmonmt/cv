#include "qrcode/qr_decode_utils.h"

#include "qrcode/array_walker.h"

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

  // We discard any bits left in cur_val because they're remainder
  // bits and aren't part of the QRCode.
  return out;
}
