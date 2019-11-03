#include "qrcode/qr_extract.h"

#include "absl/memory/memory.h"
#include "absl/types/optional.h"

#include "qrcode/pixel_iterator.h"
#include "qrcode/qr_extract_utils.h"
#include "qrcode/qr_utils.h"
#include "qrcode/runner.h"

absl::variant<std::unique_ptr<QRImage>, std::string> NormalizeCode(
    cv::Mat image, const LocatedCode& located_code) {
  cv::Mat rotation_matrix = cv::getRotationMatrix2D(
      cv::Point2f(located_code.center.x, located_code.center.y),
      -located_code.rotation_angle, 1.0);

  cv::Mat rotated_image;
  cv::warpAffine(image, rotated_image, rotation_matrix,
                 {image.cols, image.rows});

  // Rotation introduces gray, so we have to threshold again.
  // NOTE: This needs to use the same threshold as the threshold() call in
  // qr_main.cc, which is awkward.
  cv::threshold(rotated_image, rotated_image, 127, 255, cv::THRESH_BINARY);

  PixelIterator<const unsigned char> iter(
      rotated_image.ptr<uchar>(0), rotated_image.cols, rotated_image.rows);

  // The positioning points in located_code are pre-rotation. First we transform
  // them using the rotation matrix so we know where they are post-rotation.
  // We'll end up with points in the center positioning point box, but they
  // might be off-center due to translation error or (more likely) difficulties
  // detecting centers pre-rotation. Recenter the points by looking at their
  // positions in the positioning point boxes.

  auto update_point = [&](const Point& point) {
    std::vector<cv::Point2d> vec = {cv::Point2d(point.x, point.y)};
    cv::transform(vec, vec, rotation_matrix);
    Point transformed_point(vec[0].x, vec[0].y);
    return RecenterPositioningPoint(transformed_point, iter);
  };

  PositioningPoints points = located_code.positioning_points;
  points.top_left = update_point(points.top_left);
  points.top_right = update_point(points.top_right);
  points.bottom_left = update_point(points.bottom_left);

  auto qr_code = absl::make_unique<QRImage>();
  qr_code->image = rotated_image;
  qr_code->positioning_points = points;
  qr_code->center = CalculateCodeCenter(points);

  return qr_code;
}
