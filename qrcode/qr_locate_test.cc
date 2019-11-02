#include "qrcode/qr_locate.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {

using ::testing::_;
using ::testing::DoubleNear;
using ::testing::Eq;
using ::testing::VariantWith;

constexpr char kTestImageRelPath[] = "qrcode/testdata/qrcode_tilt1_small.jpg";

TEST(LocateCodeTest, Test) {
  cv::Mat input = cv::imread(kTestImageRelPath, cv::IMREAD_COLOR);
  ASSERT_TRUE(input.data != nullptr);

  cv::Mat gray, out;
  cv::cvtColor(input, gray, cv::COLOR_BGR2GRAY);
  cv::threshold(gray, out, 127, 255, cv::THRESH_BINARY);

  auto result = LocateCode(out);
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
