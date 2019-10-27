#ifndef _QRCODE_QR_H_
#define _QRCODE_QR_H_ 1

#include <iostream>
#include <vector>

#include "absl/types/optional.h"
#include "absl/types/variant.h"
#include "opencv2/opencv.hpp"

#include "qrcode/point.h"
#include "qrcode/qr_types.h"

bool IsPositioningBlock(const std::vector<int>& lens);

// Cluster the set of input points. thresh is the maximum Manhattan
// distance allowed between the first point in a cluster and any
// subsequent point. max_clusters is the maximum number of clusters
// allowed. If the number of clusters founds exceeds this number the
// function will return early.
absl::optional<std::vector<Point>> ClusterPoints(const std::vector<Point>& in,
                                                 int thresh, int max_clusters);

// Orders three points such that points 1 and 2 form a line that is
// perpendicular to the line formed by points 2 and 3.
absl::optional<PositioningPoints> OrderPositioningPoints(const Point& a,
                                                         const Point& b,
                                                         const Point& c);

// Calculate the angle of rotation of the code relative to upright.
double CalculateCodeRotationAngle(const PositioningPoints& points);

// Calculate the center of the code from the positioning points.
Point CalculateCodeCenter(const PositioningPoints& points);

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

#endif  // _QRCODE_QR_H_
