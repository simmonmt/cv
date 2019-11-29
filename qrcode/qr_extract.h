#ifndef _QRCODE_QR_EXTRACT_H_
#define _QRCODE_QR_EXTRACT_H_ 1

#include <memory>
#include <vector>

#include "absl/types/variant.h"

#include "qrcode/qr_normalize.h"
#include "qrcode/qr_types.h"

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

absl::variant<std::unique_ptr<QRCodeArray>, std::string> ExtractCode(
    const QRImage& qr_image);

#endif  // _QRCODE_QR_EXTRACT_H_
