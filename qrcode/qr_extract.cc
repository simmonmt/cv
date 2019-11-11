#include "qrcode/qr_extract.h"

#include <tuple>

#include "absl/types/optional.h"

#include "qrcode/debug_image.h"
#include "qrcode/pixel_iterator.h"
#include "qrcode/runner.h"

namespace {

std::tuple<int, int> LastRunPos(int start, const std::vector<int>& runs) {
  for (int i = 0; i < runs.size(); i++) {
    start += runs[i];
  }

  return std::make_tuple(start, runs[runs.size() - 1]);
}

// Locates the outside ring of a positioning point, in a given
// direction. Returns the position offset (relative to center) and the
// width/height (as appropriate) of the ring at that point.
//
// delta_x and delta_y must be {-1,0,-1}, and one must be zero.
absl::optional<std::tuple<int, int>> GetPositioningOuterBorder(
    PixelIterator<const uchar> iter, const Point& center, int delta_x,
    int delta_y) {
  iter.Seek(center);
  const absl::optional<std::vector<int>> maybe_result =
      Runner(DirectionalIterator<const uchar>(iter, delta_y, delta_x))
          .Next(2, nullptr);

  if (!maybe_result.has_value()) {
    return absl::nullopt;
  }

  const std::vector<int> runs = *maybe_result;
  int off = 0;
  for (int i = 0; i < runs.size(); i++) {
    off += runs[i];
  }
  return std::make_tuple(off, runs[runs.size() - 1]);
};

absl::optional<std::tuple<int, int>> GetPositioningOuterBorderDown(
    PixelIterator<const uchar> iter, const Point& center) {
  return GetPositioningOuterBorder(iter, center, 0, 1);
}

}  // namespace

absl::variant<std::string> ExtractCode(const QRImage& qr_image) {
  PixelIterator<const uchar> iter = PixelIteratorFromGrayImage(qr_image.image);

  // The timing marks are positioned as follows relative to the top
  // left positionining mark.
  //
  //      XXXXXXX
  //      X     X
  //      X XXX X
  //      X XXX X
  //      X XXX X
  //      X     X
  //      XXXXXXX T T T T
  //
  //            T
  //
  //            T
  //
  // Find the top timing marks, which means first finding where they
  // start and end (start being the last X before the horizontal T's
  // above). Then all black runs between them are timing marks. We
  // could also just find the start then accumulate only short timing
  // marks, assuming that the first long one is the top right
  // positioning mark. This seems both fiddly and fragile.

  auto result =
      GetPositioningOuterBorderDown(iter, qr_image.positioning_points.top_left);
  if (!result.has_value()) {
    return "failed to find h timing y";
  }

  int y_off, y_h;
  std::tie(y_off, y_h) = *result;

  int h_timing_y = qr_image.positioning_points.top_left.y + y_off + y_h / 2;

  iter.Seek(qr_image.positioning_points.top_left.x, h_timing_y);
  int h_timing_left_x =
      qr_image.positioning_points.top_left.x +
      (*Runner(iter.MakeForwardColumnIterator()).Next(1, nullptr))[0];

  iter.Seek(qr_image.positioning_points.top_right.x, h_timing_y);
  int h_timing_right_x =
      qr_image.positioning_points.top_right.x -
      (*Runner(iter.MakeReverseColumnIterator()).Next(1, nullptr))[0];

  // // Find horizontal timing marks start x and y from top left pp
  // iter.Seek(qr_image.positioning_points.top_left);
  // const absl::optional<std::vector<int>> maybe_result =
  //     Runner(iter.MakeForwardRowIterator()).Next(2, nullptr);
  // if (!maybe_result.has_value()) {
  //   return "failed to find h timing y";
  // }

  // int run_start_y, run_h;
  // std::tie(run_start_y, run_h) =
  //     LastRunPos(qr_image.positioning_points.top_left.y, *maybe_result);

  // int h_timing_y = run_start_y + run_h / 2;

  // iter.Seek(qr_image.positioning_points.top_left.x, h_timing_y);
  // int x_off;
  // if (!Runner(iter.MakeForwardColumnIterator()).Next(1, &x_off).has_value())
  // {
  //   return "failed to find h timing left x";
  // }
  // int h_timing_left_x = qr_image.positioning_points.top_left.x + x_off;

  auto debug = DebugImage::FromGray(qr_image.image);
  debug->Crosshairs(Point(h_timing_left_x, h_timing_y));
  debug->Crosshairs(Point(h_timing_right_x, h_timing_y));
  cv::imwrite("/tmp/img.png", debug->Mat());

  // Find horizontal timing marks end y from top right pp
  return "";
}
