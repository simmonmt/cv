#include "qrcode/qr_attributes.h"

#include <iostream>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {

using ::testing::_;
using ::testing::ElementsAreArray;
using ::testing::VariantWith;

void VerifyTypeMap(int version, const std::vector<std::string>& expected) {
  auto result = QRAttributes::New(version, QRECC_M);
  ASSERT_TRUE(absl::holds_alternative<std::unique_ptr<QRAttributes>>(result))
      << absl::get<std::string>(result);
  auto attributes = std::move(absl::get<std::unique_ptr<QRAttributes>>(result));

  std::vector<std::string> out;
  out.resize(attributes->modules_per_side());
  for (int y = 0; y < attributes->modules_per_side(); y++) {
    std::string* line = &out[y];
    line->resize(attributes->modules_per_side());
    for (int x = 0; x < attributes->modules_per_side(); x++) {
      (*line)[x] = QRAttributes::ModuleTypeToChar(
          attributes->GetModuleType(Point(x, y)));
    }
  }

  std::cout << "Got for version " << version << "\n";
  for (int i = 0; i < out.size(); ++i) {
    std::cout << i << ":\t" << out[i] << "\n";
  }

  EXPECT_THAT(out, ElementsAreArray(expected));
}

TEST(QRAttributesTest, TypeMapV1) {
  const std::vector<std::string> expected = {
      "PPPPPPPPF....PPPPPPPP",  //
      "PPPPPPPPF....PPPPPPPP",  //
      "PPPPPPPPF....PPPPPPPP",  //
      "PPPPPPPPF....PPPPPPPP",  //
      "PPPPPPPPF....PPPPPPPP",  //
      "PPPPPPPPF....PPPPPPPP",  //
      "PPPPPPPPTTTTTPPPPPPPP",  //
      "PPPPPPPPF....PPPPPPPP",  //
      "FFFFFFTFF....FFFFFFFF",  //
      "......T..............",  //
      "......T..............",  //
      "......T..............",  //
      "......T..............",  //
      "PPPPPPPPF............",  //
      "PPPPPPPPF............",  //
      "PPPPPPPPF............",  //
      "PPPPPPPPF............",  //
      "PPPPPPPPF............",  //
      "PPPPPPPPF............",  //
      "PPPPPPPPF............",  //
      "PPPPPPPPF............",  //
  };

  VerifyTypeMap(1, expected);
}

TEST(QRAttributesTest, TypeMapV2) {
  const std::vector<std::string> expected = {
      "PPPPPPPPF........PPPPPPPP",  //
      "PPPPPPPPF........PPPPPPPP",  //
      "PPPPPPPPF........PPPPPPPP",  //
      "PPPPPPPPF........PPPPPPPP",  //
      "PPPPPPPPF........PPPPPPPP",  //
      "PPPPPPPPF........PPPPPPPP",  //
      "PPPPPPPPTTTTTTTTTPPPPPPPP",  //
      "PPPPPPPPF........PPPPPPPP",  //
      "FFFFFFTFF........FFFFFFFF",  //
      "......T..................",  //
      "......T..................",  //
      "......T..................",  //
      "......T..................",  //
      "......T..................",  //
      "......T..................",  //
      "......T..................",  //
      "......T.........AAAAA....",  //
      "PPPPPPPPF.......AAAAA....",  //
      "PPPPPPPPF.......AAAAA....",  //
      "PPPPPPPPF.......AAAAA....",  //
      "PPPPPPPPF.......AAAAA....",  //
      "PPPPPPPPF................",  //
      "PPPPPPPPF................",  //
      "PPPPPPPPF................",  //
      "PPPPPPPPF................",  //
  };

  VerifyTypeMap(2, expected);
}

TEST(QRAttributesTest, OtherMethods) {
  EXPECT_THAT(QRAttributes::New(7, QRECC_L), VariantWith<std::string>(_));
  EXPECT_THAT(QRAttributes::New(99, QRECC_L), VariantWith<std::string>(_));

  auto result = QRAttributes::New(5, QRECC_H);
  ASSERT_TRUE(absl::holds_alternative<std::unique_ptr<QRAttributes>>(result))
      << absl::get<std::string>(result);
  auto attributes = std::move(absl::get<std::unique_ptr<QRAttributes>>(result));

  EXPECT_EQ(5, attributes->version());
  EXPECT_EQ(QRECC_H, attributes->ecc_level());
  EXPECT_EQ(37, attributes->modules_per_side());

  const QRErrorLevelCharacteristics error_characteristics =
      attributes->error_characteristics();
  ASSERT_EQ(2, error_characteristics.block_sets.size());
  EXPECT_EQ(33, error_characteristics.block_sets[0].block_codewords);
}

}  // namespace
