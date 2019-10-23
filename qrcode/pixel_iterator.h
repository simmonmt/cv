#ifndef _QRCODE_PIXEL_ITERATOR_
#define _QRCODE_PIXEL_ITERATOR_ 1

template <class T> class PixelIterator;

template <class T>
class DirectionalIterator {
 public:
  DirectionalIterator(PixelIterator<T> iter, int row_delta, int col_delta)
      : iter_(iter),
        row_delta_(row_delta),
        col_delta_(col_delta) {}
  virtual ~DirectionalIterator() = default;

  bool Next() {
    return iter_.RelSeekRowCol(row_delta_, col_delta_);
  }

  T Get() {
    return iter_.Get();
  }

 private:
  PixelIterator<T> iter_;
  int row_delta_, col_delta_;
};

template <class T>
class PixelIterator {
 public:
  PixelIterator(T* data, int width, int height)
      : row_(0),
        col_(0),
        cur_(0),
        width_(width),
        height_(height),
        data_(data) {}
  virtual ~PixelIterator() = default;

  bool SeekRowCol(int row, int col) {
    if (row < 0 || row >= height_) {
      return false;
    }

    if (col < 0 || col >= width_) {
      return false;
    }

    row_ = row;
    col_ = col;
    cur_ = row_ * width_ + col_;
    return true;
  }

  bool RelSeekRowCol(int row, int col) {
    return SeekRowCol(row_ + row, col_ + col);
  }

  T Get() { return data_[cur_]; }

  DirectionalIterator<T> MakeForwardRowIterator() { return DirectionalIterator<T>(*this, 1, 0); }
  DirectionalIterator<T> MakeReverseRowIterator() { return DirectionalIterator<T>(*this, -1, 0); }
  DirectionalIterator<T> MakeForwardColumnIterator() { return DirectionalIterator<T>(*this, 0, 1); }
  DirectionalIterator<T> MakeReverseColumnIterator() { return DirectionalIterator<T>(*this, 0, -1); }

 private:
  int row_, col_, cur_;
  int width_, height_;
  const T* data_;
};

#endif  // _QRCODE_PIXEL_ITERATOR_
