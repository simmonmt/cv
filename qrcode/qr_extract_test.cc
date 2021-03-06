#include "qrcode/qr_extract.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "qrcode/qr_array.h"
#include "qrcode/testutils.h"

namespace {

constexpr char kTestImageRelPath[] = "qrcode/testdata/straight.png";
constexpr char kTestBitsRelPath[] = "qrcode/testdata/straight.txt";

TEST(ExtractCode, Test) {
  QRImage qr_image;
  qr_image.image = cv::imread(kTestImageRelPath, cv::IMREAD_GRAYSCALE);
  ASSERT_TRUE(qr_image.image.data != nullptr);

  qr_image.positioning_points.top_left = Point(669, 683);
  qr_image.positioning_points.top_right = Point(1526, 677);
  qr_image.positioning_points.bottom_left = Point(672, 1542);
  qr_image.center = Point(1099, 1110);

  ASSIGN_OR_ASSERT(std::unique_ptr<QRCodeArray> array, ExtractCode(qr_image),
                   "extract returned error");

  ASSIGN_OR_ASSERT(std::unique_ptr<QRCodeArray> expected_array,
                   ReadQRCodeArrayFromFile(kTestBitsRelPath),
                   "read returned error");

  ASSERT_EQ(expected_array->height(), array->height());
  ASSERT_EQ(expected_array->width(), array->width());
  for (int y = 0; y < expected_array->height(); ++y) {
    for (int x = 0; x < expected_array->width(); ++x) {
      Point p(x, y);
      ASSERT_EQ(expected_array->Get(p), array->Get(p)) << p;
    }
  }
}

}  // namespace
