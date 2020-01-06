#ifndef _QRCODE_ARRAY_WALKER_H_
#define _QRCODE_ARRAY_WALKER_H_ 1

#include "absl/types/optional.h"

#include "qrcode/point.h"

// A helper class that iterates through the coordinates in an array
// (typically a QRCodeArray) in the order that matches the bit layout
// in QR codes.
class ArrayWalker {
 public:
  ArrayWalker(int w, int h);
  ~ArrayWalker() = default;

  // Returns the next point, or nullopt if we've exhausted the array.
  absl::optional<Point> Next();

 private:
  Point loc_;
  int h_;
  bool up_;
  absl::optional<Point> queue_;
};

#endif  // _QRCODE_ARRAY_WALKER_H_
