#ifndef _QRCODE_QR_EXTRACT_H_
#define _QRCODE_QR_EXTRACT_H_ 1

#include "absl/types/variant.h"
#include "opencv2/opencv.hpp"

#include "qrcode/point.h"
#include "qrcode/qr_locate.h"
#include "qrcode/qr_types.h"

// Routines to extract a normalized version of a QR code from an image.

struct QRCode {
  cv::Mat image;
  PositioningPoints positioning_points;
  Point center;
};

absl::variant<std::unique_ptr<QRCode>, std::string> ExtractCode(
    cv::Mat image, const LocatedCode& located_code);

#endif  // _QRCODE_QR_EXTRACT_H_
