#ifndef _QRCODE_QR_ARRAY_H_
#define _QRCODE_QR_ARRAY_H_ 1

#include <vector>

#include "qrcode/point.h"

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

#endif  // _QRCODE_QR_ARRAY_H_
