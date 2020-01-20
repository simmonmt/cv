#include "qrcode/bch_new.h"

#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
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

TEST_F(DecodeBCHTest, MTS) {
  GF16 gf;
  const int c = 1, d = 7;
  std::vector<bool> ref = {0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1};

  std::vector<bool> in = ref;
  in[13] = !in[13];
  in[0] = !in[0];

  EXPECT_THAT(RunTest(gf, in, c, d),
              VariantWith<std::vector<bool>>(ElementsAreArray(ref)));
  EXPECT_THAT(RunOldTest(gf, in, c, d),
              VariantWith<std::vector<bool>>(ElementsAreArray(ref)));
}

// Tests 0- through 3-bit errors
TEST_F(DecodeBCHTest, Exhaustive) {
  GF16 gf;
  const int c = 1, d = 7;

  std::vector<std::string> failures;
  int num_successes = 0;

  std::vector<bool> ref = {0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1};
  for (int i = 0; i <= ref.size(); ++i) {
    for (int j = 0; j <= ref.size(); ++j) {
      for (int k = 0; k <= ref.size(); ++k) {
        if (i == j || j == k) {
          continue;
        }

        std::vector<bool> in = ref;
        std::vector<int> flipped;
        auto flip = [&](int d) {
          in[d] = !in[d];
          flipped.push_back(d);
        };

        if (i < ref.size()) flip(i);
        if (j < ref.size()) flip(j);
        if (k < ref.size()) flip(k);

        auto result = RunTest(gf, in, c, d);
        if (!absl::holds_alternative<std::vector<bool>>(result) ||
            absl::get<std::vector<bool>>(result) != ref) {
          failures.push_back(absl::StrJoin(flipped, ","));
        } else {
          ++num_successes;
        }
      }
    }
  }

  std::cout << "#successes " << num_successes << " #failures "
            << failures.size() << "\n";
  if (!failures.empty()) {
    FAIL() << "failures: " << ::testing::PrintToString(failures) << "\n";
  }
}

}  // namespace
