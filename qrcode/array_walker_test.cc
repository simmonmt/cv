#include "qrcode/array_walker.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "qrcode/testutils.h"

namespace {

using ::testing::ElementsAreArray;

TEST(ArrayWalkerTest, Test) {
  ASSIGN_OR_ASSERT(std::unique_ptr<QRAttributes> attributes,
                   QRAttributes::New(2, QRECC_L), "error with QRAttributes");

  // This tests up-to-down, down-to-up, and termination on up.
  ArrayWalker walker(*attributes);

  std::vector<Point> got;
  for (int i = 0;; ++i) {
    auto p = walker.Next();
    if (!p.has_value()) {
      break;
    }
    got.push_back(*p);

    if (i > 1000) {
      ASSERT_TRUE(false) << "unexpected non-termination";
    }
  }

  auto p = [](int x, int y) { return Point(x, y); };

  auto verify_range = [&](int start, int len,
                          const std::vector<Point>& expected) {
    std::cout << "verify_range(start=" << start << ", len=" << len << ")\n";
    EXPECT_THAT(
        std::vector<Point>(got.begin() + start, got.begin() + start + len),
        ElementsAreArray(expected))
        << "start " << start;
  };

  // 0-7 (block 1). Verify that we start in the right place and that we're doing
  // an upward block in the right order.
  verify_range(0, 8,
               {
                   p(24, 24), p(23, 24),  // 76
                   p(24, 23), p(23, 23),  // 54
                   p(24, 22), p(23, 22),  // 32
                   p(24, 21), p(23, 21),  // 10
               });

  // 24-39 (blocks 4 and 5). This verifies a premature up-to-down transition
  // (premature because the positioning block is in the way).
  verify_range(24, 16,
               {
                   p(24, 12), p(23, 12),  // 76
                   p(24, 11), p(23, 11),  // 54
                   p(24, 10), p(23, 10),  // 32
                   p(24, 9), p(23, 9),    // 10

                   p(22, 9), p(21, 9),    // 76
                   p(22, 10), p(21, 10),  // 54
                   p(22, 11), p(21, 11),  // 32
                   p(22, 12), p(21, 12),  // 10
               });

  // 56-79 (blocks 8-10). This verifies a down-to-up transition at the bottom
  // edge whose up starts at the bottom as well as an upward complete
  // interruption (the alignment mark).
  verify_range(56, 24,
               {
                   p(22, 21), p(21, 21),  // 76
                   p(22, 22), p(21, 22),  // 54
                   p(22, 23), p(21, 23),  // 32
                   p(22, 24), p(21, 24),  // 10

                   p(20, 24), p(19, 24),  // 76
                   p(20, 23), p(19, 23),  // 54
                   p(20, 22), p(19, 22),  // 32
                   p(20, 21), p(19, 21),  // 10

                   p(20, 15), p(19, 15),  // 76
                   p(20, 14), p(19, 14),  // 54
                   p(20, 13), p(19, 13),  // 32
                   p(20, 12), p(19, 12),  // 10
               });

  // 96-103 (block 13). This verifies a downward complete interruption (the
  // alignment mark).
  verify_range(96, 8,
               {
                   p(18, 14), p(17, 14),  // 76
                   p(18, 15), p(17, 15),  // 54
                   p(18, 21), p(17, 21),  // 32
                   p(18, 22), p(17, 22),  // 10
               });

  // 112-127 (blocks 15-16). This verifies an upward partial interruption (the
  // left side of the alignment mark).
  verify_range(112, 16,
               {
                   p(16, 22), p(15, 22),  // 76
                   p(16, 21), p(15, 21),  // 54
                   p(15, 20),             // 3
                   p(15, 19),             // 2
                   p(15, 18),             // 1
                   p(15, 17),             // 0

                   p(15, 16),             // 7
                   p(16, 15), p(15, 15),  // 65
                   p(16, 14), p(15, 14),  // 43
                   p(16, 13), p(15, 13),  // 21
                   p(16, 12),             // 0
               });

  // 288-295 (block 37). This verifies a down-to-up transition at the bottom
  // edge whose up doesn't start at the bottom.
  verify_range(288, 8,
               {
                   p(9, 21),             // 7
                   p(10, 22), p(9, 22),  // 65
                   p(10, 23), p(9, 23),  // 43
                   p(10, 24), p(9, 24),  // 21
                   p(8, 16),             // 0
               });

  // 304-311 (block 39). This verifies a premature up-to-down transition that
  // spans the verical timing pattern.
  verify_range(304, 8,
               {
                   p(7, 12),                   // 7
                   p(8, 11), p(7, 11),         // 65
                   p(8, 10), p(7, 10),         // 43
                   p(8, 9), p(7, 9), p(5, 9),  // 210
               });

  // 320-327 (block 41). This verifies a premature down-to-up transition.
  verify_range(320, 8,
               {
                   p(4, 13),                      // 7
                   p(5, 14), p(4, 14),            // 65
                   p(5, 15), p(4, 15),            // 43
                   p(5, 16), p(4, 16), p(3, 16),  // 210
               });

  // 344-351 (block 44). This verifies the final block.
  verify_range(344, 8,
               {
                   p(0, 9),             // 7
                   p(1, 10), p(0, 10),  // 65
                   p(1, 11), p(0, 11),  // 43
                   p(1, 12), p(0, 12),  // 21
                   p(1, 13),            // 0
               });

  // The code stops at block 44, but the walker doesn't know that and will
  // return the partial "block 45".
  EXPECT_EQ(359, got.size());
}

}  // namespace
