#ifndef _QRCODE_QR_LOCATE_H_
#define _QRCODE_QR_LOCATE_H_ 1

#include <memory>

#include "absl/types/variant.h"
#include "opencv2/opencv.hpp"

#include "qrcode/point.h"
#include "qrcode/qr_types.h"

// Describes the location and orientation of a QR code in an image.
struct LocatedCode {
  PositioningPoints positioning_points;
  Point center;
  double rotation_angle;
};

// Attempts to locate a single QR code in an image. Returns a string error
// message if no QR code can be found.
absl::variant<std::unique_ptr<LocatedCode>, std::string> LocateCode(
    cv::Mat image);

#endif  // _QRCODE_QR_LOCATE_H_
