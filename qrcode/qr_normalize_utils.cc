#include "qrcode/qr_normalize_utils.h"

#include "qrcode/runner.h"

Point RecenterPositioningPoint(const Point& point,
                               PixelIterator<const unsigned char> iter) {
  auto measure = [](DirectionalIterator<const unsigned char> iter) {
    auto result = Runner(iter).Next(1, nullptr);
    if (result.has_value()) {
      return (*result)[0];
    } else {
      return -1;
    }
  };

  iter.Seek(point);
  int left_margin = measure(iter.MakeReverseColumnIterator());
  int right_margin = measure(iter.MakeForwardColumnIterator());
  int top_margin = measure(iter.MakeReverseRowIterator());
  int bottom_margin = measure(iter.MakeForwardRowIterator());

  Point out = point;

  int h_diff = left_margin - right_margin;
  if (h_diff > 1 || h_diff < -1) {
    out.x -= h_diff / 2;
  }

  int v_diff = top_margin - bottom_margin;
  if (v_diff > 1 || v_diff < -1) {
    out.y -= v_diff / 2;
  }

  return out;
}
