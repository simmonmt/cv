#include "qrcode/runner.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {

using ::testing::ElementsAre;
using ::testing::ElementsAreArray;
using ::testing::Eq;
using ::testing::Optional;

std::vector<unsigned char> MakeRun(std::vector<int> lens) {
  std::vector<unsigned char> out;
  for (int i = 0; i < lens.size(); ++i) {
    const int len = lens[i];
    std::vector<unsigned char> run(len, (i % 2 == 0) ? 255 : 0);
    out.insert(out.end(), run.begin(), run.end());
  }
  return out;
}

TEST(MakeRunTest, Test) {
  auto out = MakeRun({1, 2, 0, 1, 4});
  EXPECT_THAT(out, ElementsAreArray({255, 0, 0, 0, 255, 255, 255, 255}));
}

class RunnerTest : public ::testing::Test {};

TEST_F(RunnerTest, Test) {
  const std::vector<unsigned char> run =
      MakeRun({1, 2, 3, 4, 5, 6, 7, 8, 9, 10});
  Runner runner(absl::Span<const unsigned char>(run.data(), run.size()));

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
}

}  // namespace
