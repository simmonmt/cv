#include "qrcode/qr_locate_utils.h"

#include <assert.h>
#include <cmath>
#include <vector>

#include "absl/base/macros.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"

#include "qrcode/pixel_iterator.h"
#include "qrcode/qr_types.h"
#include "qrcode/runner.h"

bool IsPositioningBlock(const std::vector<int>& lens) {
  const int lb = lens[0];
  const int lw = lens[1];
  const int c = lens[2];
  const int rw = lens[3];
  const int rb = lens[4];

  // NOTE: These thresholds (0.5-1.5 for the small, 2.5-3.5 for the large) are
  // from the reference decode algorithm in the spec.

  double small_avg = (lb + lw + rw + rb) / 4.0;
  for (int n : {lb, lw, rw, rb}) {
    double sz = n / small_avg;
    if (sz < 0.5 || sz > 1.5) {
      return false;
    }
  }

  double big_normalized = c / small_avg;
  if (big_normalized < 2.5 || big_normalized > 3.5) {
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

std::vector<Point> FindPositioningPointCandidatesInRow(
    PixelIterator<const unsigned char>* image_iter, int row) {
  image_iter->Seek(0, row);

  // If the row starts with white we need to skip the first set of
  // values returned by the runner.
  bool skip_first = image_iter->Get() != 0;

  Runner runner(image_iter->MakeForwardColumnIterator());
  std::vector<Point> candidates;

  if (skip_first) {
    runner.Next(1, nullptr);
  }

  for (;;) {
    int h_start_x;
    auto result = runner.Next(5, &h_start_x);
    if (result == absl::nullopt) {
      return candidates;
    }

    const std::vector<int> lens = std::move(result.value());
    if (IsPositioningBlock(lens)) {
      const int left_black_width = lens[0];
      const int left_white_width = lens[1];
      const int center_width = lens[2];

      int center_x =
          h_start_x + left_black_width + left_white_width + center_width / 2;

      image_iter->Seek(center_x, row);

      // {row, center_x} is in the middle of a run of black, in the middle of a
      // series of runs that's compatible with the ratios for a positioning
      // block. If we're truly in the middle of a positioning block, we can
      // confirm that by looking for a series of runs that are compatible with a
      // positioning block on a vertical line that runs through {row, center_x}.
      //
      // Because we know it has to go through {row, center_x}, we can start from
      // that point, looking for black-white-black in either direction. In both
      // cases we'll be looking *out*, and starting from black, so the first
      // black runs we find are actually part of the same run. Join the two
      // together and we can check for positioning ratios.
      //
      // A picture, rotated 90 degrees:
      //
      //     (up)      aaaaa     bbbb+bbbbbbbbbb     ccccc      (down)
      //
      // The horizontal line search got us to +, which is {row, center_x}. Our
      // search up from + finds black run B, then a white run, then black run
      // A. Our search down from + finds the rest of black run B, then a white
      // run, then black run C. Before we check for positioning block ratios we
      // need to combine the results of both searches so we have lengths for
      // black run A, the white run, black run B (which is the sum of the black
      // run Bs from the two searches), the other white run, and black run C. We
      // therefore do a ratio check on this:
      //
      //     (up)      aaaaa     bbbbbbbbbbbbbbb     ccccc      (down)
      //
      // If the ratio check succeeds, center_y is in the middle of run B (which
      // may not be the same as the location of +).
      auto get_three = [](DirectionalIterator<const unsigned char> iter) {
        return Runner(iter).Next(3, nullptr);
      };

      absl::optional<std::vector<int>> maybe_three_up =
          get_three(image_iter->MakeReverseRowIterator());
      absl::optional<std::vector<int>> maybe_three_down =
          get_three(image_iter->MakeForwardRowIterator());

      if (maybe_three_up.has_value() && maybe_three_down.has_value()) {
        const std::vector<int> three_up = std::move(maybe_three_up.value());
        const std::vector<int> three_down = std::move(maybe_three_down.value());
        const int center_height = three_up[0] + three_down[0];

        std::vector<int> combined = {
            three_up[2],    // top black height
            three_up[1],    // top white height
            center_height,  // center height
            three_down[1],  // bottom white height
            three_down[2],  // bottow black height
        };

        if (IsPositioningBlock(combined)) {
          const int center_y = row - three_up[0] + center_height / 2;
          candidates.emplace_back(center_x, center_y);
        }
      }
    }

    // The next group starts with white, which is no good to us. Skip it.
    runner.Next(1, nullptr);
  }
}
