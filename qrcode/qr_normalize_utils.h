#ifndef _QRCODE_QR_NORMALIZE_UTILS_H_
#define _QRCODE_QR_NORMALIZE_UTILS_H_ 1

#include "qrcode/pixel_iterator.h"
#include "qrcode/qr_types.h"

// Given a point that's roughly in the middle of the center black box
// of a positioning point, recenter it by measuring the distances to
// the horizontal and vertical edges of that box. Returns the
// recentered point.
Point RecenterPositioningPoint(const Point& point,
                               PixelIterator<const unsigned char> iter);

#endif  // _QRCODE_QR_NORMALIZE_UTILS_H_
