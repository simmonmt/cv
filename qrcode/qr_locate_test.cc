#include "qrcode/qr_locate.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {

using ::testing::_;
using ::testing::DoubleNear;
using ::testing::Eq;
using ::testing::VariantWith;

constexpr char kTestImageRelPath[] = "qrcode/testdata/tilt.png";

TEST(LocateCodeTest, Test) {
  cv::Mat image = cv::imread(kTestImageRelPath, cv::IMREAD_GRAYSCALE);
  ASSERT_TRUE(image.data != nullptr);

  auto result = LocateCode(image);
  ASSERT_THAT(result, VariantWith<std::unique_ptr<LocatedCode>>(_));
  std::unique_ptr<LocatedCode> located_code =
      std::move(absl::get<std::unique_ptr<LocatedCode>>(result));

  PositioningPoints expected_points = {
      {1016, 514},
      {1705, 1014},
      {506, 1203},
  };

  EXPECT_THAT(located_code->positioning_points, Eq(expected_points));
  EXPECT_THAT(located_code->center, Eq(Point(1105, 1109)));
  EXPECT_THAT(located_code->rotation_angle, DoubleNear(-36.5, 0.25));
}

}  // namespace
