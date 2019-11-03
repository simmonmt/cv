#include "qrcode/qr_locate_utils.h"

#include "absl/base/macros.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "qrcode/testutils.h"

namespace {

using ::testing::ElementsAre;
using ::testing::ElementsAreArray;
using ::testing::Eq;
using ::testing::Optional;

PositioningPoints MakePositioningPoints(const Point& top_left,
                                        const Point& top_right,
                                        const Point& bottom_left) {
  PositioningPoints pp;
  pp.top_left = top_left;
  pp.top_right = top_right;
  pp.bottom_left = bottom_left;
  return pp;
}

TEST(IsPositioningBlockTest, Test) {
  struct TestCase {
    std::vector<int> lens;
    bool expected;
  };

  static const TestCase kTestCases[] = {
      {{5, 5, 15, 5, 5}, false},     {{15, 18, 50, 16, 17}, true},
      {{50, 16, 17, 16, 49}, false}, {{42, 53, 142, 53, 42}, true},
      {{31, 39, 103, 37, 33}, true}, {{34, 36, 105, 36, 34}, true},
  };

  for (int i = 0; i < ABSL_ARRAYSIZE(kTestCases); ++i) {
    const TestCase& tc = kTestCases[i];
    EXPECT_EQ(tc.expected, IsPositioningBlock(tc.lens)) << "case " << i;
  }
}

TEST(ClusterPointsTest, Simple) {
  static std::vector<Point> kPoints = {{10, 10},  //
                                       {11, 11},  // 2 away from 10, 10
                                       {11, 12},  // 3 away from 10, 10
                                       {20, 20},  //
                                       {30, 30}};

  // Just right
  EXPECT_THAT(
      ClusterPoints(kPoints, 5, 99),
      Optional(ElementsAre(Point(10, 10), Point(20, 20), Point(30, 30))));

  // Too many clusters
  EXPECT_THAT(ClusterPoints(kPoints, 5, 2), Eq(absl::nullopt));
}

TEST(ClusterPointsTest, Large) {
  static std::vector<Point> kPoints = {
      {1582, 909},  {1582, 909},  {1581, 908},  {1581, 908},  {1581, 908},
      {1580, 907},  {1579, 907},  {1578, 906},  {1577, 906},  {1577, 906},
      {1576, 904},  {1576, 904},  {1575, 903},  {1574, 903},  {1573, 903},
      {1573, 903},  {1572, 901},  {1571, 901},  {1570, 899},  {1570, 899},
      {1569, 897},  {1567, 896},  {2271, 1410}, {2271, 1410}, {2270, 1408},
      {2270, 1408}, {2269, 1408}, {2268, 1408}, {2267, 1407}, {2267, 1407},
      {2266, 1406}, {2265, 1406}, {2265, 1406}, {2264, 1405}, {2264, 1405},
      {2263, 1404}, {2262, 1404}, {2262, 1404}, {2261, 1404}, {2260, 1403},
      {2259, 1403}, {2259, 1403}, {1072, 1598}, {1071, 1598}, {1070, 1598},
      {1070, 1598}, {1070, 1598}, {1069, 1597}, {1068, 1596}, {1067, 1596},
      {1066, 1595}, {1064, 1593}, {1064, 1593}, {1064, 1593}, {1063, 1593},
      {1062, 1592}, {1061, 1592}, {1060, 1591}, {1060, 1591}, {1059, 1590},
      {1058, 1590}, {1058, 1590}, {1058, 1590}};

  EXPECT_THAT(ClusterPoints(kPoints, 50, 3),
              Optional(ElementsAre(Point(1582, 909), Point(2271, 1410),
                                   Point(1072, 1598))));
}

class OrderPositioningPointsTest : public ::testing::Test {
 public:
  static bool PointLess(const Point& a, const Point& b) {
    if (a.y < b.y) {
      return true;
    } else if (a.y > b.y) {
      return false;
    } else {
      return a.x < b.x;
    }
  }
};

TEST_F(OrderPositioningPointsTest, Static) {
  static const PositioningPoints kTestCases[] = {
      // Ninety degree angles
      MakePositioningPoints({50, 50}, {100, 50}, {50, 100}),
      // Synthetic values whose slope difference should be ~0. This set can
      // tolerate a diff up to 0.9, but works fine with 0.1.
      MakePositioningPoints({19998, 9825}, {10174, 19998}, {9825, 1}),
      // Actual values from qrcode_tilt1.jpg. This set can tolerate a diff
      // up to 0.8, but works fine with 0.1.
      MakePositioningPoints({1582, 909}, {2271, 1410}, {1072, 1598}),
      // Actual values from qrcode1_small.jpg.
      MakePositioningPoints({392, 525}, {1163, 539}, {367, 1296}),
  };

  for (const PositioningPoints& expected : kTestCases) {
    std::cout << "== trying " << expected;

    std::vector<Point> points = {expected.top_left, expected.top_right,
                                 expected.bottom_left};

    std::sort(points.begin(), points.end(), PointLess);
    for (int i = 0;; ++i) {
      EXPECT_THAT(OrderPositioningPoints(points[0], points[1], points[2]),
                  Optional(expected))
          << "order " << points[0] << " " << points[1] << " " << points[2];
      if (!std::next_permutation(points.begin(), points.end(), PointLess)) {
        break;
      }
    }
  }
}

TEST_F(OrderPositioningPointsTest, Exhaustive) {
  const Point center = {100, 100};
  for (const PositioningPoints expected :
       MakeRotatedPositioningPoints(center, 50)) {
    std::vector<Point> points = {expected.top_left, expected.top_right,
                                 expected.bottom_left};

    std::sort(points.begin(), points.end(), PointLess);
    for (int i = 0;; ++i) {
      EXPECT_THAT(OrderPositioningPoints(points[0], points[1], points[2]),
                  Optional(expected))
          << "order " << points[0] << " " << points[1] << " " << points[2];
      if (!std::next_permutation(points.begin(), points.end(), PointLess)) {
        break;
      }
    }
  }
}

TEST(CalculateRotationAngleTest, Test) {
  EXPECT_EQ(90.0, CalculateCodeRotationAngle(MakePositioningPoints(
                      Point(100, 50), Point(100, 100), Point(50, 50))));
  EXPECT_EQ(-90.0, CalculateCodeRotationAngle(MakePositioningPoints(
                       Point(50, 50), Point(50, 0), Point(100, 50))));
}

}  // namespace
