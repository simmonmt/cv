#include "qrcode/qr_extract.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "qrcode/qr_locate.h"

namespace {

using ::testing::_;
using ::testing::Eq;
using ::testing::VariantWith;

constexpr char kTestImageRelPath[] = "qrcode/testdata/tilt.png";

TEST(ExtractCodeTest, Test) {
  cv::Mat image = cv::imread(kTestImageRelPath, cv::IMREAD_GRAYSCALE);
  ASSERT_TRUE(image.data != nullptr);

  std::unique_ptr<LocatedCode> located_code =
      std::move(absl::get<std::unique_ptr<LocatedCode>>(LocateCode(image)));

  // If this fails, the output of LocateCode has changed, which means
  // assertion failures from ExtractCode results are likely the fault
  // of LocateCode changes -- not problems with ExtractCode.
  ASSERT_THAT(located_code->center, Eq(Point(1105, 1109)));

  auto extract_result = ExtractCode(image, *located_code);
  ASSERT_THAT(extract_result, VariantWith<std::unique_ptr<QRCode>>(_));
  std::unique_ptr<QRCode> qr_code =
      std::move(absl::get<std::unique_ptr<QRCode>>(extract_result));

  PositioningPoints expected_points = {
      {669, 683},
      {1526, 677},
      {672, 1542},
  };
  EXPECT_THAT(qr_code->positioning_points, Eq(expected_points));

  // The center moves, not because of the rotation, but because ExtractCode
  // recenters the positioning points.
  EXPECT_THAT(qr_code->center, Eq(Point(1099, 1110)));
}

}  // namespace
