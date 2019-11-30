#ifndef _QRCODE_QR_TYPES_H_
#define _QRCODE_QR_TYPES_H_ 1

#include <iostream>
#include <string>
#include <vector>

#include "qrcode/point.h"

// The centers of positioning points whose orientation (relative to
// each other) has been discovered.
struct PositioningPoints {
  Point top_left;
  Point top_right;
  Point bottom_left;

  bool operator==(const PositioningPoints& p) const {
    return p.top_left == top_left && p.top_right == top_right &&
           p.bottom_left == bottom_left;
  }
};

std::ostream& operator<<(std::ostream& stream, const PositioningPoints& pp);

// An array of true/false points representing an extracted
// QRCode. True means black, false means white.
class QRCodeArray {
 public:
  QRCodeArray(int height, int width);

  int height() const { return height_; }
  int width() const { return width_; }

  void Set(Point p, bool val);
  bool Get(Point p) const;

  void Dump() const;

 private:
  int height_, width_;
  std::vector<bool> array_;
};

#endif  // _QRCODE_QR_TYPES_H_
