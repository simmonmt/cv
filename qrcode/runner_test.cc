#include "qrcode/runner.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "qrcode/testutils.h"

namespace {

using ::testing::ElementsAre;
using ::testing::Eq;
using ::testing::Optional;

class RunnerTest : public ::testing::Test {};

TEST_F(RunnerTest, Test) {
  const std::vector<unsigned char> run =
      MakeRun({1, 2, 3, 4, 5, 6, 7, 8, 9, 10});
  PixelIterator<const unsigned char> iter(run.data(), run.size(), 1);
  Runner runner(iter.MakeForwardColumnIterator());

  int idx;
  absl::optional<std::vector<int>> result;

  result = runner.Next(7, &idx);
  ASSERT_THAT(result, Optional(ElementsAre(1, 2, 3, 4, 5, 6, 7)));
  EXPECT_EQ(0, idx);

  result = runner.Next(3, &idx);
  ASSERT_THAT(result, Optional(ElementsAre(2, 3, 4)));
  EXPECT_EQ(1, idx);

  result = runner.Next(8, &idx);
  ASSERT_THAT(result, Optional(ElementsAre(3, 4, 5, 6, 7, 8, 9, 10)));
  EXPECT_EQ(3, idx);

  result = runner.Next(8, &idx);
  ASSERT_THAT(result, Eq(absl::nullopt));

  // Reset the runner
  runner = Runner(iter.MakeForwardColumnIterator());

  // Make sure we can call it without an index pointer.
  result = runner.Next(7, nullptr);
  ASSERT_THAT(result, Optional(ElementsAre(1, 2, 3, 4, 5, 6, 7)));

  // Make sure we can get the last run -- the one that doesn't have
  // anything to the right. Start by skipping to the end.
  for (int i = 2; i <= 9; i++) {
    runner.Next(1, nullptr);
  }
  result = runner.Next(1, nullptr);
  ASSERT_THAT(result, Optional(ElementsAre(10)));
}

}  // namespace
