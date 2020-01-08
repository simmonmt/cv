#include "qrcode/qr_decode.h"

#include <string>

#include "absl/types/variant.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "qrcode/qr_array.h"
#include "qrcode/testutils.h"

namespace {

using ::testing::ElementsAreArray;

constexpr char kTestStraightRelPath[] = "qrcode/testdata/straight.txt";
constexpr char kTestDataSpecExamplePath[] =
    "qrcode/testdata/spec_example_1m.txt";

class QRDecodeTest : public ::testing::Test {};

TEST_F(QRDecodeTest, Metadata) {
  ASSIGN_OR_ASSERT(std::unique_ptr<QRCodeArray> array,
                   ReadQRCodeArrayFromFile(kTestStraightRelPath),
                   "read returned error");

  auto result = Decode(std::move(array));
  ASSERT_FALSE(absl::holds_alternative<std::string>(result))
      << absl::get<std::string>(result);

  std::unique_ptr<QRCode> qrcode =
      std::move(absl::get<std::unique_ptr<QRCode>>(result));

  EXPECT_EQ(29, qrcode->attributes->modules_per_side());
  EXPECT_EQ(3, qrcode->attributes->version());
  EXPECT_EQ(QRECC_L, qrcode->attributes->ecc_level());
}

TEST_F(QRDecodeTest, SpecExample) {
  ASSIGN_OR_ASSERT(std::unique_ptr<QRCodeArray> array,
                   ReadQRCodeArrayFromFile(kTestDataSpecExamplePath),
                   "read returned error");

  auto result = Decode(std::move(array));
  ASSERT_FALSE(absl::holds_alternative<std::string>(result))
      << absl::get<std::string>(result);
  std::unique_ptr<QRCode> qrcode =
      std::move(absl::get<std::unique_ptr<QRCode>>(result));

  EXPECT_THAT(
      qrcode->codewords,
      ElementsAreArray({0b00010000, 0b00100000, 0b00001100, 0b01010110,
                        0b01100001, 0b10000000, 0b11101100, 0b00010001,
                        0b11101100, 0b00010001, 0b11101100, 0b00010001,
                        0b11101100, 0b00010001, 0b11101100, 0b00010001}));
}

}  // namespace
