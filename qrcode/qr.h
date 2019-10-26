#ifndef _QRCODE_QR_H_
#define _QRCODE_QR_H_ 1

#include <vector>

#include "absl/types/optional.h"

#include "qrcode/point.h"

bool IsPositioningBlock(const std::vector<int>& lens);

// Cluster the set of input points. thresh is the maximum Manhattan
// distance allowed between the first point in a cluster and any
// subsequent point. max_clusters is the maximum number of clusters
// allowed. If the number of clusters founds exceeds this number the
// function will return early.
absl::optional<std::vector<Point>> ClusterPoints(const std::vector<Point>& in,
                                                 int thresh, int max_clusters);

#endif  // _QRCODE_QR_H_
