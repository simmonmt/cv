#include "qrcode/qr_normalize.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "qrcode/qr_locate.h"

namespace {

using ::testing::_;
using ::testing::Eq;
using ::testing::VariantWith;

constexpr char kTestImageRelPath[] = "qrcode/testdata/tilt.png";

TEST(NormalizeCodeTest, Test) {
  cv::Mat image = cv::imread(kTestImageRelPath, cv::IMREAD_GRAYSCALE);
  ASSERT_TRUE(image.data != nullptr);

  std::unique_ptr<LocatedCode> located_code =
      std::move(absl::get<std::unique_ptr<LocatedCode>>(LocateCode(image)));

  // If this fails, the output of LocateCode has changed, which means
  // assertion failures from NormalizeCode results are likely the fault
  // of LocateCode changes -- not problems with NormalizeCode.
  ASSERT_THAT(located_code->center, Eq(Point(1107, 1110)));

  auto extract_result = NormalizeCode(image, *located_code);
  ASSERT_THAT(extract_result, VariantWith<std::unique_ptr<QRImage>>(_));
  std::unique_ptr<QRImage> qr_image =
      std::move(absl::get<std::unique_ptr<QRImage>>(extract_result));

  PositioningPoints expected_points = {
      {669, 684},
      {1526, 678},
      {671, 1543},
  };
  EXPECT_THAT(qr_image->positioning_points, Eq(expected_points));

  // The center moves, not because of the rotation, but because NormalizeCode
  // recenters the positioning points.
  EXPECT_THAT(qr_image->center, Eq(Point(1098, 1111)));
}

}  // namespace
