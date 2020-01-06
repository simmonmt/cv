#include "qrcode/qr_decode.h"

#include <string>

#include "absl/types/variant.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "qrcode/qr_array.h"
#include "qrcode/testutils.h"

namespace {

constexpr char kTestStraightRelPath[] = "qrcode/testdata/straight.txt";

class QRDecodeTest : public ::testing::Test {
 public:
  void SetUp() override {
    ASSIGN_OR_ASSERT(array_, ReadQRCodeArrayFromFile(kTestStraightRelPath),
                     "read returned error");
  }

  std::unique_ptr<QRCodeArray> array_;
};

TEST_F(QRDecodeTest, Metadata) {
  auto result = Decode(std::move(array_));
  ASSERT_FALSE(absl::holds_alternative<std::string>(result))
      << absl::get<std::string>(result);

  std::unique_ptr<QRCode> qrcode =
      std::move(absl::get<std::unique_ptr<QRCode>>(result));

  EXPECT_EQ(29, qrcode->attributes->modules_per_side());
  EXPECT_EQ(3, qrcode->attributes->version());
  EXPECT_EQ(QRECC_M, qrcode->attributes->ecc_level());
}

}  // namespace
