#include "qrcode/qr.h"

#include "absl/base/macros.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {

using ::testing::ElementsAre;
using ::testing::Eq;
using ::testing::Optional;

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

TEST(ClusterPointsTest, Test) {
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

}  // namespace
