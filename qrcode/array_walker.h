#ifndef _QRCODE_ARRAY_WALKER_H_
#define _QRCODE_ARRAY_WALKER_H_ 1

#include "absl/types/optional.h"

#include "qrcode/point.h"
#include "qrcode/qr_attributes.h"

// A helper class that iterates through the coordinates in an array
// (typically a QRCodeArray) in the order that matches the bit layout
// in QR codes.
class ArrayWalker {
 public:
  // The passed QRAttributes instance must survive this class.
  ArrayWalker(const QRAttributes& attributes);
  ~ArrayWalker() = default;

  // Returns the next point, or nullopt if we've exhausted the array.
  absl::optional<Point> Next();

 private:
  const QRAttributes& attr_;
  Point loc_;
  int h_;
  bool up_;
  absl::optional<Point> queue_;
};

#endif  // _QRCODE_ARRAY_WALKER_H_
