#include "qrcode/array_walker.h"

#include <iostream>

ArrayWalker::ArrayWalker(const QRAttributes& attributes)
    : attr_(attributes),
      loc_(Point(attributes.modules_per_side() - 1,
                 attributes.modules_per_side() - 1)),
      h_(attributes.modules_per_side()),
      up_(true) {}

absl::optional<Point> ArrayWalker::Next() {
  if (queue_.has_value()) {
    Point out = *queue_;
    queue_.reset();
    return out;
  }

  if (loc_.x < 0 || loc_.y < 0) {
    return absl::nullopt;
  }

  auto is_data = [&](const Point& p) {
    return attr_.GetModuleType(p) == QRAttributes::TYPE_DATA;
  };

  auto pair_has_data = [&](const Point& p) {
    return is_data(p) || is_data(Point(p.x - 1, p.y));
  };

  // One or both of the pair will be a data cell. If only one, return
  // that directly (put it in 'out' so it'll be returned at the end of
  // the function). If both, return the right one (loc) and queue the
  // left one (loc.x-1,loc.y).
  //
  // In practice the various interruptions are laid out such that it
  // doesn't look like we'll ever see a right without a left, but
  // we'll be defensive and pretend that could happen.
  Point out;
  if (is_data(loc_)) {
    Point pair(loc_.x - 1, loc_.y);
    if (is_data(pair)) {
      queue_ = pair;
    }
    out = loc_;
  } else {
    out = Point(loc_.x - 1, loc_.y);
  }

  if (up_) {
    // Find the next pair *upwards* that has a data module for either
    // element.
    do {
      --loc_.y;
    } while (loc_.y >= 0 && !pair_has_data(loc_));

    if (loc_.y < 0) {
      // We advanced off the top edge, so position ourselves to go
      // *downwards*.
      up_ = false;

      // Shift to the next pair to the left. If that shift puts the
      // right side of the pair (i.e. loc) on the vertical timing
      // pattern, shift by another module. This somewhat hacky way of
      // finding the timing pattern (we hard code a y that's just
      // beyond the top left positioning mark) is a lot simpler than
      // trying to deal with it later when we're adjusting y.
      loc_.x -= 2;
      if (attr_.GetModuleType(Point(loc_.x, 10)) ==
          QRAttributes::TYPE_TIMING_PATTERN) {
        --loc_.x;
      }

      if (loc_.x >= 0) {
        // We didn't advance off the left edge, so find the first pair
        // downwards that has a data module for either element.
        loc_.y = 0;
        while (!pair_has_data(loc_) && loc_.y < h_) {
          ++loc_.y;
        }
      }
    }

  } else {
    // Find the next pair *downwards* that has a data module for
    // either element. The algorithm here is the same as for upwards,
    // with some increments and bounds checks reversed.
    do {
      ++loc_.y;
    } while (loc_.y < h_ && !pair_has_data(loc_));

    if (loc_.y >= h_) {
      up_ = true;

      loc_.x -= 2;
      if (attr_.GetModuleType(Point(loc_.x, 10)) ==
          QRAttributes::TYPE_TIMING_PATTERN) {
        --loc_.x;
      }

      if (loc_.x >= 0) {
        loc_.y = h_ - 1;
        while (!pair_has_data(loc_) && loc_.y >= 0) {
          --loc_.y;
        }
      }
    }
  }

  return out;
}
