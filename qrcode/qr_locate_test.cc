#include "qrcode/qr_locate.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {

using ::testing::_;
using ::testing::DoubleNear;
using ::testing::Eq;
using ::testing::VariantWith;

constexpr char kTiltImageRelPath[] = "qrcode/testdata/tilt.png";
constexpr char kStraightImageRelPath[] = "qrcode/testdata/straight.png";

class LocateCodeTest : public ::testing::Test {
 public:
  void TestImage(const std::string& path, const PositioningPoints& points,
                 const Point& center, double rotation_angle) {
    cv::Mat image = cv::imread(path, cv::IMREAD_GRAYSCALE);
    ASSERT_TRUE(image.data != nullptr) << path;

    auto result = LocateCode(image);
    ASSERT_THAT(result, VariantWith<std::unique_ptr<LocatedCode>>(_))
        << "had string: " << absl::get<std::string>(result);
    std::unique_ptr<LocatedCode> located_code =
        std::move(absl::get<std::unique_ptr<LocatedCode>>(result));

    EXPECT_THAT(located_code->positioning_points, Eq(points));
    EXPECT_THAT(located_code->center, Eq(center));
    EXPECT_THAT(located_code->rotation_angle,
                DoubleNear(rotation_angle, 0.025));
  }
};

TEST_F(LocateCodeTest, Straight) {
  PositioningPoints expected_points = {
      {668, 684},
      {1526, 677},
      {672, 1542},
  };

  Point expected_center(1099, 1110);
  double expected_angle = 0.267;

  TestImage(kStraightImageRelPath, expected_points, expected_center,
            expected_angle);
}

TEST_F(LocateCodeTest, Tilt) {
  PositioningPoints expected_points = {
      {1015, 513},
      {1710, 1018},
      {506, 1203},
  };

  Point expected_center(1107, 1110);
  double expected_angle = -36.4;

  TestImage(kTiltImageRelPath, expected_points, expected_center,
            expected_angle);
}

}  // namespace
