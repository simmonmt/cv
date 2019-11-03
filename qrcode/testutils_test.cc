#include "qrcode/testutils.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {

using ::testing::ElementsAreArray;

TEST(MakeRunTest, Test) {
  auto out = MakeRun({1, 2, 0, 1, 4});
  EXPECT_THAT(out, ElementsAreArray({255, 0, 0, 0, 255, 255, 255, 255}));
}

}  // namespace
