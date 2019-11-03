#ifndef _QRCODE_TESTUTILS_H_
#define _QRCODE_TESTUTILS_H_ 1

#include <vector>

#include "qrcode/qr_types.h"

std::vector<unsigned char> MakeRun(std::vector<int> lens);

// Returns a set of positioning points at all angles.
//
// Given a center and a radius, this function will construct a series of
// positioning points, each of which are at a different rotation relative to the
// center. The returned points will represent ~all possible orientations of
// positioning points.
//
// NOTE: radius must not be greater than center.x or center.y.
std::vector<PositioningPoints> MakeRotatedPositioningPoints(const Point& center,
                                                            double radius);

#endif  // _QRCODE_TESTUTILS_H_