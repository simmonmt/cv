#include "qrcode/qr_extract.h"

#include "gtest/gtest.h"

namespace {

constexpr char kTestImageRelPath[] = "qrcode/testdata/straight.png";

TEST(ExtractCode, Test) {
  QRImage qr_image;
  qr_image.image = cv::imread(kTestImageRelPath, cv::IMREAD_GRAYSCALE);
  ASSERT_TRUE(qr_image.image.data != nullptr);

  qr_image.positioning_points.top_left = Point(669, 683);
  qr_image.positioning_points.top_right = Point(1526, 677);
  qr_image.positioning_points.bottom_left = Point(672, 1542);
  qr_image.center = Point(1099, 1110);

  ExtractCode(qr_image);
}

}  // namespace
