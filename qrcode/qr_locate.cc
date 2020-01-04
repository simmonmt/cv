#include "qrcode/qr_locate.h"

#include <assert.h>
#include <cmath>
#include <vector>

#include "absl/base/macros.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"

#include "qrcode/pixel_iterator.h"
#include "qrcode/qr_locate_utils.h"
#include "qrcode/qr_utils.h"
#include "qrcode/runner.h"

absl::variant<std::unique_ptr<LocatedCode>, std::string> LocateCode(
    cv::Mat image) {
  PixelIterator<const uchar> image_iter = PixelIteratorFromGrayImage(image);
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
