#include "qrcode/qr_array.h"

QRCodeArray::QRCodeArray(int height, int width)
    : height_(height), width_(width), array_(height * width) {}

void QRCodeArray::Set(Point p, bool val) {
  if (p.x < 0 || p.y < 0 || p.x >= width_ || p.y >= height_) {
    return;
  }

  array_[p.y * width_ + p.x] = val;
}

bool QRCodeArray::Get(Point p) const {
  if (p.x < 0 || p.y < 0 || p.x >= width_ || p.y >= height_) {
    return false;
  }

  return array_[p.y * width_ + p.x];
}

QRCodeArray::PointType QRCodeArray::Type(Point p) const {}

void QRCodeArray::Dump() const {
  int num_rows = 0;
  int div = 1;
  for (int n = width_; n > 0; n /= 10) {
    if (++num_rows > 1) {
      div *= 10;
    }
  }

  while (div >= 1) {
    for (int x = 0; x < width_; ++x) {
      std::cout << (x / div) % 10;
    }
    div /= 10;
    std::cout << "\n";
  }

  for (int y = 0; y < height_; ++y) {
    for (int x = 0; x < width_; ++x) {
      Point p(x, y);
      std::cout << (Get(p) ? "X" : " ");
    }
    std::cout << "  // " << y << "\n";
  }
}
