#ifndef _QRCODE_PIXEL_ITERATOR_
#define _QRCODE_PIXEL_ITERATOR_ 1

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

  PixelIterator(const PixelIterator&) = delete;

  bool SeekRow(int row) {
    if (row < 0 || row >= height_) {
      return false;
    }

    row_ = row;
    cur_ = row_ * width_ + col_;
    return true;
  }

  bool SeekCol(int col) {
    if (col < 0 || col >= width_) {
      return false;
    }

    col_ = col;
    cur_ = row_ * width_ + col_;
    return true;
  }

  T Get() { return data_[cur_]; }

  bool NextRow() {
    int next = row_ + 1;
    if (next >= height_) {
      return false;
    }

    cur_ += width_;
    return true;
  }

  bool NextCol() {
    int next = col_ + 1;
    if (next >= width_) {
      return false;
    }

    ++cur_;
    return true;
  }

 private:
  int row_, col_, cur_;
  int width_, height_;
  const T* data_;
};

template <class T>
class ConstrainedIterator {
 public:
  virtual ~ConstrainedIterator() = default;

  virtual bool Next(T* p) = 0;
};

template <class T>
class RowIterator : public ConstrainedIterator<T> {
 public:
  RowIterator(PixelIterator<T>* iter) : iter_(iter) {}
  ~RowIterator() = default;

  bool Next(T* p) override { return iter_->NextCol(p); }

 private:
  PixelIterator<T>* iter_;
};

template <class T>
class ColIterator : public ConstrainedIterator<T> {
 public:
  ColIterator(PixelIterator<T>* iter) : iter_(iter) {}
  ~ColIterator() = default;

  bool Next(T* p) override { return iter_->NextRow(p); }

 private:
  PixelIterator<T>* iter_;
};

#endif  // _QRCODE_PIXEL_ITERATOR_
