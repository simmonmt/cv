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

// A helper macro that simplifies access to absl::variant<foo, std::string>
// instances used as value-or-error returns from functions. If `code` returns a
// variant with the non-string set, that value will be assigned to `target`. If
// the variant contains a string, it'll assume that an error has been returned
// by `code`, and will invoke a gtest assertion failure.
//
// Inspired and partially copied from http://github.com/google/mediapipe and
// adapted for a (sad, sad) world without StatusOr.
#define ASSIGN_OR_ASSERT(target, code, msg)                                    \
  ASSIGN_OR_ASSERT_IMPL_(STATUS_MACROS_IMPL_CONCAT_(status_variant, __LINE__), \
                         target, code, msg)

#define ASSIGN_OR_ASSERT_IMPL_(status, target, code, msg)      \
  auto status = code;                                          \
  if (status.index() == 1) {                                   \
    ASSERT_TRUE(false) << msg << ": " << absl::get<1>(status); \
  }                                                            \
  target = std::move(absl::get<0>(status));

// For some reason __LINE__ doesn't get evaluated unless we nest the macros like
// this.
#define STATUS_MACROS_IMPL_CONCAT_INNER_(x, y) x##y
#define STATUS_MACROS_IMPL_CONCAT_(x, y) STATUS_MACROS_IMPL_CONCAT_INNER_(x, y)

#endif  // _QRCODE_TESTUTILS_H_
