#include "qrcode/qr.h"

#include <assert.h>
#include <cmath>
#include <vector>

#include "absl/base/macros.h"
#include "absl/strings/str_cat.h"

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

namespace {

bool TryOrder(const Point& a, const Point& b, const Point& c) {
  double rise_atob = static_cast<double>(b.y - a.y);
  double run_atob = static_cast<double>(b.x - a.x);
  double slope_atob = rise_atob / run_atob;

  double rise_btoc = static_cast<double>(c.y - b.y);
  double run_btoc = static_cast<double>(c.x - b.x);
  double slope_btoc = rise_btoc / run_btoc;

  // Are they at right angles to each other?
  //
  // Take the reciprocal of the slope with the larger absolute value
  // so we're making consistent comparisons regardless of order. The
  // values we get now as a and b will later arrive in b and a. Using
  // consistent ordering also makes it possible to use a consistent
  // threshold.
  double diff;
  if (std::abs(slope_atob) > std::abs(slope_btoc)) {
    diff = std::abs((-1.0 / slope_atob) - slope_btoc);
  } else {
    diff = std::abs(slope_atob - (-1.0 / slope_btoc));
  }
  // std::cout << a << " " << b << " " << c << " diff " << diff << "\n";
  if (diff >= 0.1) {
    return false;
  }

  // Is it a right turn from atob to btoc? Take the cross product of
  // the two (made a little more annoying because we're using a,b,c to
  // refer to points while most descriptions use a and b for the
  // vectors).
  double dir = run_atob * rise_btoc - run_btoc * rise_atob;
  // std::cout << a << " " << b << " " << c << " dir " << dir << "\n";
  return dir > 0;
}

}  // namespace

std::ostream& operator<<(std::ostream& stream, const PositioningPoints& pp) {
  return stream << "PP<tl:" << pp.top_left << " tr:" << pp.top_right
                << " bl:" << pp.bottom_left << ">";
}

absl::optional<PositioningPoints> OrderPositioningPoints(const Point& a,
                                                         const Point& b,
                                                         const Point& c) {
  std::vector<const Point*> in = {&a, &b, &c};

  // We want an ordering of points such that the slope of the line
  // described by points a and b is the negative reciprocal of the
  // slope of the line described by points b and c.
  static const int kOrders[][3] = {
      {0, 1, 2}, {0, 2, 1}, {1, 0, 2}, {1, 2, 0}, {2, 0, 1}, {2, 1, 0},
  };

  for (int i = 0; i < ABSL_ARRAYSIZE(kOrders); ++i) {
    const Point& a = *in[kOrders[i][0]];
    const Point& b = *in[kOrders[i][1]];
    const Point& c = *in[kOrders[i][2]];

    if (TryOrder(a, b, c)) {
      PositioningPoints points;
      points.bottom_left = a;
      points.top_left = b;
      points.top_right = c;
      return points;
    }
  }

  return absl::nullopt;
}

double CalculateCodeRotationAngle(const PositioningPoints& points) {
  const double rise = points.top_left.y - points.bottom_left.y;
  const double run = points.top_left.x - points.bottom_left.x;

  return std::atan(run / rise) / (2 * M_PI) * 360.0;
}

Point CalculateCodeCenter(const PositioningPoints& points) {
  double atob_half_xoff = (points.top_left.x - points.bottom_left.x) / 2;
  double atob_half_yoff = (points.top_left.y - points.bottom_left.y) / 2;

  double btoc_half_xoff = (points.top_right.x - points.top_left.x) / 2;
  double btoc_half_yoff = (points.top_right.y - points.top_left.y) / 2;

  return Point(points.bottom_left.x + atob_half_xoff + btoc_half_xoff,
               points.bottom_left.y + atob_half_yoff + btoc_half_yoff);
}
