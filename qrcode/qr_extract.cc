#include "qrcode/qr_extract.h"

#include "absl/memory/memory.h"

absl::variant<std::unique_ptr<QRCode>, std::string> ExtractCode(
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

  auto qr_code = absl::make_unique<QRCode>();
  qr_code->image = rotated_image;

  // The positioning points in located_code are pre-rotation. Transform them
  // using the rotation matrix so we know where they are post-rotation.
  auto transform_point = [&](const Point& point) {
    std::vector<cv::Point2d> vec = {cv::Point2d(point.x, point.y)};
    cv::transform(vec, vec, rotation_matrix);
    return Point(vec[0].x, vec[0].y);
  };

  qr_code->positioning_points.top_left =
      transform_point(located_code.positioning_points.top_left);
  qr_code->positioning_points.top_right =
      transform_point(located_code.positioning_points.top_right);
  qr_code->positioning_points.bottom_left =
      transform_point(located_code.positioning_points.bottom_left);
  qr_code->center = transform_point(located_code.center);

  return qr_code;
}
