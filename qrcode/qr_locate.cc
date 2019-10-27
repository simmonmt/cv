#include "qrcode/qr_locate.h"

#include <assert.h>
#include <cmath>
#include <vector>

#include "absl/base/macros.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"

#include "qrcode/pixel_iterator.h"
#include "qrcode/runner.h"

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

std::vector<Point> FindPositioningPointCandidatesInRow(
    PixelIterator<const uchar>* image_iter, int row) {
  image_iter->SeekRowCol(row, 0);

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

      image_iter->SeekRowCol(row, center_x);

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
      auto get_three = [](DirectionalIterator<const uchar> iter) {
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

absl::variant<std::unique_ptr<LocatedCode>, std::string> LocateCode(
    cv::Mat image) {
  PixelIterator<const uchar> image_iter(image.ptr<uchar>(0), image.cols,
                                        image.rows);
  std::vector<Point> candidates;
  for (int row = 0; row < image.rows; ++row) {
    auto row_candidates = FindPositioningPointCandidatesInRow(&image_iter, row);
    candidates.insert(candidates.end(), row_candidates.begin(),
                      row_candidates.end());
  }

  if (candidates.size() < 3) {
    return "failed to find 3 positioning blocks";
  }

  // The threshold for clustering candidate partioning block centers. We can get
  // away with 10 if the code is already properly oriented, but we need a bigger
  // threshold to cover angled cases where our ratio detection might have a
  // harder time.
  constexpr int kPositioningBlockClusteringThreshold = 50;

  absl::optional<std::vector<Point>> maybe_clusters =
      ClusterPoints(candidates, kPositioningBlockClusteringThreshold, 3);
  if (!maybe_clusters.has_value()) {
    return "clustering failed: too many clusters";
  } else if (maybe_clusters->size() != 3) {
    return absl::StrFormat("clustering failed: wanted 3 clusters, got %d",
                           maybe_clusters->size());
  }

  const std::vector<Point> clusters = std::move(maybe_clusters.value());
  std::cout << "#clustered candidates found: " << clusters.size() << "\n";

  absl::optional<PositioningPoints> maybe_positioning_points =
      OrderPositioningPoints(clusters[0], clusters[1], clusters[2]);
  if (!maybe_positioning_points.has_value()) {
    return "failed to find correct ordering";
  }

  auto located_code = absl::make_unique<LocatedCode>();
  located_code->positioning_points =
      std::move(maybe_positioning_points.value());
  located_code->center = CalculateCodeCenter(located_code->positioning_points);
  located_code->rotation_angle =
      CalculateCodeRotationAngle(located_code->positioning_points);

  return std::move(located_code);
}
