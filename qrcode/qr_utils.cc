#include "qrcode/qr_utils.h"

Point CalculateCodeCenter(const PositioningPoints& points) {
  double atob_half_xoff = (points.top_left.x - points.bottom_left.x) / 2;
  double atob_half_yoff = (points.top_left.y - points.bottom_left.y) / 2;

  double btoc_half_xoff = (points.top_right.x - points.top_left.x) / 2;
  double btoc_half_yoff = (points.top_right.y - points.top_left.y) / 2;

  return Point(points.bottom_left.x + atob_half_xoff + btoc_half_xoff,
               points.bottom_left.y + atob_half_yoff + btoc_half_yoff);
}
