#ifndef _QRCODE_TESTUTILS_H_
#define _QRCODE_TESTUTILS_H_ 1

#include <string>
#include <vector>

#include "absl/types/variant.h"

#include "qrcode/qr_array.h"
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

// Return a QRCodeArray from a file or return an error message explaining the
// failure. Unset cells are spaces; set cells are 'X'.
absl::variant<std::unique_ptr<QRCodeArray>, std::string>
ReadQRCodeArrayFromFile(const std::string& path);

// Return a QRCodeArray generated from an array of strings. Unset cells are
// spaces; set cells are 'X'.
std::unique_ptr<QRCodeArray> ReadQRCodeArrayFromStrings(
    const std::vector<std::string> strs);

// Renders a QRCodeArray as strings. Unset cells are spaces, set cells are 'x's.
std::vector<std::string> QRCodeArrayToStrings(const QRCodeArray& array);

#endif  // _QRCODE_TESTUTILS_H_
