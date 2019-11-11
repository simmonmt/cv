#ifndef _QRCODE_PIXEL_ITERATOR_
#define _QRCODE_PIXEL_ITERATOR_ 1

#include "qrcode/qr_types.h"

namespace cv {
class Mat;
}  // namespace cv

template <class T>
class PixelIterator;

template <class T>
class DirectionalIterator {
 public:
  DirectionalIterator(PixelIterator<T> iter, int row_delta, int col_delta)
      : iter_(iter), row_delta_(row_delta), col_delta_(col_delta) {}
  virtual ~DirectionalIterator() = default;

  bool Next() { return iter_.RelSeek(col_delta_, row_delta_); }

  T Get() { return iter_.Get(); }

 private:
  PixelIterator<T> iter_;
  int row_delta_, col_delta_;
};

template <class T>
class PixelIterator {
 public:
  PixelIterator(T* data, int width, int height)
      : x_(0), y_(0), cur_(0), width_(width), height_(height), data_(data) {}
  virtual ~PixelIterator() = default;

  bool Seek(int x, int y) {
    if (y < 0 || y >= height_) {
      return false;
    }

    if (x < 0 || x >= width_) {
      return false;
    }

    y_ = y;
    x_ = x;
    cur_ = y_ * width_ + x_;
    return true;
  }

  bool RelSeek(int delta_x, int delta_y) {
    return Seek(x_ + delta_x, y_ + delta_y);
  }

  bool Seek(Point p) { return Seek(p.x, p.y); }

  T Get() { return data_[cur_]; }

  DirectionalIterator<T> MakeForwardRowIterator() {
    return DirectionalIterator<T>(*this, 1, 0);
  }
  DirectionalIterator<T> MakeReverseRowIterator() {
    return DirectionalIterator<T>(*this, -1, 0);
  }
  DirectionalIterator<T> MakeForwardColumnIterator() {
    return DirectionalIterator<T>(*this, 0, 1);
  }
  DirectionalIterator<T> MakeReverseColumnIterator() {
    return DirectionalIterator<T>(*this, 0, -1);
  }

 private:
  int x_, y_, cur_;
  int width_, height_;
  const T* data_;
};

PixelIterator<const unsigned char> PixelIteratorFromGrayImage(
    const cv::Mat& image);

#endif  // _QRCODE_PIXEL_ITERATOR_
