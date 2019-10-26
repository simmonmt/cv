#include "qrcode/qr.h"

#include <vector>

bool IsPositioningBlock(const std::vector<int>& lens) {
  const int lb = lens[0];
  const int lw = lens[1];
  const int c = lens[2];
  const int rw = lens[3];
  const int rb = lens[4];

  if (lb < 10) {
    return false;
  }

  // NOTE: These thresholds were determined empirically

  double small = (lb + lw + rw + rb) / 4.0;
  double small_high = small * 1.15, small_low = small * 0.85;
  for (int n : {lb, lw, rw, rb}) {
    if (n < small_low || n > small_high) {
      return false;
    }
  }

  double big_high = small * 3 * 1.05, big_low = small * 3 * 0.95;
  if (c < big_low || c > big_high) {
    return false;
  }

  return true;
}

absl::optional<std::vector<Point>> ClusterPoints(const std::vector<Point>& in,
                                                 int thresh, int max_clusters) {
  std::vector<Point> clusters;
  clusters.reserve(max_clusters);

  auto dist = [](const Point& a, const Point& b) {
    return std::abs(a.x - b.x) + std::abs(a.y - b.y);
  };

  for (const Point& point : in) {
    bool found = false;
    for (const Point& cluster : clusters) {
      if (dist(point, cluster) <= thresh) {
        found = true;
        break;  // it's in an existing cluster, so discard
      }
    }

    if (found) {
      continue;
    }

    if (clusters.size() >= max_clusters) {
      return absl::nullopt;  // too many clusters
    }

    clusters.push_back(point);
  }

  return clusters;
}
