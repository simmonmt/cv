#include "qrcode/array_walker.h"

#include <iostream>

ArrayWalker::ArrayWalker(int w, int h)
    : loc_(Point(w - 1, h - 1)), h_(h), up_(true) {}

absl::optional<Point> ArrayWalker::Next() {
  if (queue_.has_value()) {
    Point out = *queue_;
    queue_.reset();
    return out;
  }

  if (loc_.x < 0 || loc_.y < 0) {
    return absl::nullopt;
  }

  queue_ = Point(loc_.x - 1, loc_.y);
  Point out = loc_;

  if (up_) {
    if (--loc_.y < 0) {
      loc_.x -= 2;
      loc_.y = 0;
      up_ = false;
    }
  } else {
    if (++loc_.y == h_) {
      loc_.x -= 2;
      loc_.y = h_ - 1;
      up_ = true;
    }
  }

  return out;
}
