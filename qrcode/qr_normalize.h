#ifndef _QRCODE_QR_NORMALIZE_H_
#define _QRCODE_QR_NORMALIZE_H_ 1

#include "absl/types/variant.h"
#include "opencv2/opencv.hpp"

#include "qrcode/point.h"
#include "qrcode/qr_locate.h"
#include "qrcode/qr_types.h"

// A normalized (straightened) version of the QR code
struct QRImage {
  cv::Mat image;
  PositioningPoints positioning_points;
  Point center;
};

// Extract a normalized version of a QR code from an image.
absl::variant<std::unique_ptr<QRImage>, std::string> NormalizeCode(
    cv::Mat image, const LocatedCode& located_code);

#endif  // _QRCODE_QR_NORMALIZE_H_
