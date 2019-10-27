#ifndef _QRCODE_QR_LOCATE_UTILS_H_
#define _QRCODE_QR_LOCATE_UTILS_H_ 1

#include <vector>

#include "absl/types/optional.h"

#include "qrcode/pixel_iterator.h"
#include "qrcode/point.h"
#include "qrcode/qr_types.h"

bool IsPositioningBlock(const std::vector<int>& lens);

std::vector<Point> FindPositioningPointCandidatesInRow(
    PixelIterator<const unsigned char>* image_iter, int row);

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

#endif  // _QRCODE_QR_LOCATE_UTILS_H_
