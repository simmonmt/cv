#include "qrcode/qr_decode_utils.h"

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
