#include "qrcode/array_walker.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {

using ::testing::ElementsAreArray;

TEST(ArrayWalkerTest, TestTerminatesUp) {
  // This tests up-to-down, down-to-up, and termination on up.
  ArrayWalker walker(6, 3);

  std::vector<Point> got;
  for (int i = 0;; ++i) {
    auto p = walker.Next();
    if (!p.has_value()) {
      break;
    }
    got.push_back(*p);

    if (i > 100) {
      ASSERT_TRUE(false) << "unexpected non-termination";
    }
  }

  EXPECT_THAT(got, ElementsAreArray({
                       Point(5, 2), Point(4, 2),  //
                       Point(5, 1), Point(4, 1),  //
                       Point(5, 0), Point(4, 0),  //
                       Point(3, 0), Point(2, 0),  //
                       Point(3, 1), Point(2, 1),  //
                       Point(3, 2), Point(2, 2),  //
                       Point(1, 2), Point(0, 2),  //
                       Point(1, 1), Point(0, 1),  //
                       Point(1, 0), Point(0, 0),  //
                   }));
}

TEST(ArrayWalkerTest, TestTerminatesDown) {
  // This tests up-to-down and termination on down.
  ArrayWalker walker(4, 3);

  std::vector<Point> got;
  for (int i = 0;; ++i) {
    auto p = walker.Next();
    if (!p.has_value()) {
      break;
    }
    got.push_back(*p);

    if (i > 100) {
      ASSERT_TRUE(false) << "unexpected non-termination";
    }
  }

  EXPECT_THAT(got, ElementsAreArray({
                       Point(3, 2), Point(2, 2),  //
                       Point(3, 1), Point(2, 1),  //
                       Point(3, 0), Point(2, 0),  //
                       Point(1, 0), Point(0, 0),  //
                       Point(1, 1), Point(0, 1),  //
                       Point(1, 2), Point(0, 2),  //
                   }));
}

}  // namespace
