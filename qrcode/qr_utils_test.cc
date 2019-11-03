#include "qrcode/qr_utils.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "qrcode/qr_types.h"
#include "qrcode/testutils.h"

namespace {

using ::testing::AllOf;
using ::testing::Eq;
using ::testing::Ge;
using ::testing::Le;

TEST(CalculateCodeCenterTest, Static) {
  PositioningPoints points;
  points.bottom_left = Point(50, 100);
  points.top_left = Point(50, 50);
  points.top_right = Point(100, 50);

  EXPECT_THAT(CalculateCodeCenter(points), Eq(Point(75, 75)));
}

TEST(CalculateCodeCenterTest, Exhaustive) {
  const Point center = {100, 100};
  for (const PositioningPoints points :
       MakeRotatedPositioningPoints(center, 50)) {
    // Because math and rounding we won't get exactly 100,100 each time. Allow
    // [99-101] for each axis.
    Point center = CalculateCodeCenter(points);
    EXPECT_THAT(center.x, AllOf(Ge(99), Le(101))) << points;
    EXPECT_THAT(center.y, AllOf(Ge(99), Le(101))) << points;
  }
}

}  // namespace
