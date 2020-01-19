#include "qrcode/bch_new.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "qrcode/bch.h"

namespace {

using ::testing::ElementsAreArray;
using ::testing::VariantWith;

class DecodeBCHTest : public ::testing::Test {
 public:
  absl::variant<std::vector<bool>, std::string> RunTest(
      const GF& gf, const std::vector<bool>& in, int c, int d) {
    auto result = DecodeBCHNew(gf, in, c, d);
    if (absl::holds_alternative<std::string>(result)) {
      ADD_FAILURE() << "WARNING: decode failed: "
                    << absl::get<std::string>(result);
    }
    return result;
  }

  absl::variant<std::vector<bool>, std::string> RunOldTest(
      const GF& gf, const std::vector<bool>& in, int c, int d) {
    auto result = DecodeBCH(gf, in, c, d);
    if (absl::holds_alternative<std::string>(result)) {
      ADD_FAILURE() << "WARNING: decode failed: "
                    << absl::get<std::string>(result);
    }
    return result;
  }
};

TEST_F(DecodeBCHTest, NoError) {
  GF16 gf;
  const int c = 1, d = 7;
  std::vector<bool> ref = {0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1};
  EXPECT_THAT(RunTest(gf, ref, c, d),
              VariantWith<std::vector<bool>>(ElementsAreArray(ref)));
}

TEST_F(DecodeBCHTest, MTS) {
  GF16 gf;
  const int c = 1, d = 7;
  std::vector<bool> ref = {0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1};

  std::vector<bool> in = ref;
  in[13] = !in[13];
  in[5] = !in[5];

  EXPECT_THAT(RunTest(gf, in, c, d),
              VariantWith<std::vector<bool>>(ElementsAreArray(ref)));
  EXPECT_THAT(RunOldTest(gf, in, c, d),
              VariantWith<std::vector<bool>>(ElementsAreArray(ref)));
}

TEST_F(DecodeBCHTest, Single) {
  GF16 gf;
  const int c = 1, d = 7;

  std::vector<bool> ref = {0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1};
  for (int i = 0; i < ref.size(); ++i) {
    std::vector<bool> in = ref;
    EXPECT_THAT(RunTest(gf, in, c, d),
                VariantWith<std::vector<bool>>(ElementsAreArray(ref)));
  }
}

TEST_F(DecodeBCHTest, Double) {
  GF16 gf;
  const int c = 1, d = 7;

  std::vector<bool> ref = {0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1};
  for (int i = 0; i < ref.size(); ++i) {
    for (int j = 0; j < ref.size(); ++j) {
      if (i == j) {
        continue;
      }

      std::vector<bool> in = ref;
      in[i] = !in[i];
      in[j] = !in[j];

      EXPECT_THAT(RunTest(gf, in, c, d),
                  VariantWith<std::vector<bool>>(ElementsAreArray(ref)))
          << "failed for " << i << " and " << j;
    }
  }
}

}  // namespace
