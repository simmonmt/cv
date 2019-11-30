#include "qrcode/qr_extract.h"

#include <tuple>

#include "absl/memory/memory.h"
#include "absl/strings/str_format.h"
#include "absl/types/optional.h"

#include "qrcode/debug_image.h"
#include "qrcode/pixel_iterator.h"
#include "qrcode/runner.h"

namespace {

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

struct Extent {
  Extent(int start, int len) : start(start), len(len) {}

  int start;
  int len;
};

absl::optional<std::vector<Extent>> FindPositioningPointExtents(
    PixelIterator<const uchar> iter, const Point& center, bool lr) {
  iter.Seek(center);
  auto back_iterator =
      lr ? iter.MakeReverseColumnIterator() : iter.MakeReverseRowIterator();
  auto fwd_iterator =
      lr ? iter.MakeForwardColumnIterator() : iter.MakeForwardRowIterator();

  auto maybe_back = Runner(back_iterator).Next(3, nullptr);
  auto maybe_fwd = Runner(fwd_iterator).Next(3, nullptr);
  if (!maybe_back.has_value() || !maybe_fwd.has_value()) {
    return absl::nullopt;
  }

  const std::vector<int>& back = *maybe_back;
  const std::vector<int>& fwd = *maybe_fwd;

  int ref = lr ? center.x : center.y;

  Extent center_black(ref - (back[0] - 1), back[0] + fwd[0] - 1);

  Extent back_white(center_black.start - back[1], back[1]);
  Extent back_black(back_white.start - back[2], back[2]);
  Extent fwd_white(center_black.start + center_black.len, fwd[1]);
  Extent fwd_black(fwd_white.start + fwd_white.len, fwd[2]);

  Extent center_black1(center_black.start, center_black.len / 3);
  Extent center_black2(center_black1.start + center_black1.len,
                       center_black.len / 3);
  Extent center_black3(
      center_black2.start + center_black2.len,
      center_black.len - center_black1.len - center_black2.len);

  return std::vector<Extent>{
      back_black,    back_white, center_black1, center_black2,
      center_black3, fwd_white,  fwd_black,
  };
}

absl::variant<std::vector<int>, std::string> FindXCoords(
    const QRImage& qr_image) {
  PixelIterator<const uchar> iter = PixelIteratorFromGrayImage(qr_image.image);

  auto result = GetPositioningOuterBorder(
      iter, qr_image.positioning_points.top_left, 0, 1);
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

  std::vector<Extent> timings;

  iter.Seek(h_timing_left_x, h_timing_y);
  Runner runner(iter.MakeForwardColumnIterator());
  for (int x = h_timing_left_x; x <= h_timing_right_x;) {
    auto maybe_run = runner.Next(1, nullptr);
    if (!maybe_run.has_value()) {
      return "h timing y ran off end";
    }

    int len = (*maybe_run)[0];
    timings.emplace_back(x, len);
    x += len;
  }

  absl::optional<std::vector<Extent>> maybe_left_extents =
      FindPositioningPointExtents(iter, qr_image.positioning_points.top_left,
                                  true);
  if (!maybe_left_extents.has_value()) {
    return "no top left extents";
  }
  timings.insert(timings.begin(), maybe_left_extents->begin(),
                 maybe_left_extents->end());

  absl::optional<std::vector<Extent>> maybe_right_extents =
      FindPositioningPointExtents(iter, qr_image.positioning_points.top_right,
                                  true);
  if (!maybe_right_extents.has_value()) {
    return "no top right extents";
  }
  timings.insert(timings.end(), maybe_right_extents->begin(),
                 maybe_right_extents->end());

  std::vector<int> x_coords(timings.size());
  for (int i = 0; i < timings.size(); ++i) {
    x_coords[i] = timings[i].start + timings[i].len / 2;
  }

  return x_coords;
}

absl::variant<std::vector<int>, std::string> FindYCoords(
    const QRImage& qr_image) {
  PixelIterator<const uchar> iter = PixelIteratorFromGrayImage(qr_image.image);

  auto result = GetPositioningOuterBorder(
      iter, qr_image.positioning_points.top_left, 1, 0);
  if (!result.has_value()) {
    return "failed to find v timing x";
  }

  int x_off, x_w;
  std::tie(x_off, x_w) = *result;

  int v_timing_x = qr_image.positioning_points.top_left.x + x_off + x_w / 2;

  iter.Seek(v_timing_x, qr_image.positioning_points.top_left.y);
  int v_timing_top_y =
      qr_image.positioning_points.top_left.y +
      (*Runner(iter.MakeForwardRowIterator()).Next(1, nullptr))[0];

  iter.Seek(v_timing_x, qr_image.positioning_points.bottom_left.y);
  int v_timing_bottom_y =
      qr_image.positioning_points.bottom_left.y -
      (*Runner(iter.MakeReverseRowIterator()).Next(1, nullptr))[0];

  std::vector<Extent> timings;

  iter.Seek(v_timing_x, v_timing_top_y);
  Runner runner(iter.MakeForwardRowIterator());
  for (int y = v_timing_top_y; y <= v_timing_bottom_y;) {
    auto maybe_run = runner.Next(1, nullptr);
    if (!maybe_run.has_value()) {
      return "v timing x ran off end";
    }

    int len = (*maybe_run)[0];
    timings.emplace_back(y, len);
    y += len;
  }

  absl::optional<std::vector<Extent>> maybe_top_extents =
      FindPositioningPointExtents(iter, qr_image.positioning_points.top_left,
                                  false);
  if (!maybe_top_extents.has_value()) {
    return "no top left v extents";
  }
  timings.insert(timings.begin(), maybe_top_extents->begin(),
                 maybe_top_extents->end());

  absl::optional<std::vector<Extent>> maybe_bottom_extents =
      FindPositioningPointExtents(iter, qr_image.positioning_points.bottom_left,
                                  false);
  if (!maybe_bottom_extents.has_value()) {
    return "no bottom left v extents";
  }
  timings.insert(timings.end(), maybe_bottom_extents->begin(),
                 maybe_bottom_extents->end());

  std::vector<int> y_coords(timings.size());
  for (int i = 0; i < timings.size(); ++i) {
    y_coords[i] = timings[i].start + timings[i].len / 2;
  }

  return y_coords;
}

}  // namespace

absl::variant<std::unique_ptr<QRCodeArray>, std::string> ExtractCode(
    const QRImage& qr_image) {
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

  auto maybe_x_coords = FindXCoords(qr_image);
  if (absl::holds_alternative<std::string>(maybe_x_coords)) {
    return "x coords fail: " + absl::get<std::string>(maybe_x_coords);
  }
  const std::vector<int> x_coords =
      std::move(absl::get<std::vector<int>>(maybe_x_coords));

  auto maybe_y_coords = FindYCoords(qr_image);
  if (absl::holds_alternative<std::string>(maybe_y_coords)) {
    return "y coords fail: " + absl::get<std::string>(maybe_y_coords);
  }
  const std::vector<int> y_coords =
      std::move(absl::get<std::vector<int>>(maybe_y_coords));

  auto qr_array =
      absl::make_unique<QRCodeArray>(y_coords.size(), x_coords.size());

  PixelIterator<const uchar> image_iter =
      PixelIteratorFromGrayImage(qr_image.image);
  for (int y = 0; y < y_coords.size(); ++y) {
    for (int x = 0; x < x_coords.size(); ++x) {
      Point image_point(x_coords[x], y_coords[y]);
      Point qr_point(x, y);
      image_iter.Seek(image_point);
      qr_array->Set(qr_point, image_iter.Get() == 0);
    }
  }

  return std::move(qr_array);
}
