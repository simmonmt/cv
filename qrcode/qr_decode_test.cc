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
    auto result = ReadQRCodeArrayFromFile(kTestStraightRelPath);
    if (absl::holds_alternative<std::string>(result)) {
      ASSERT_TRUE(false) << "read returned error: "
                         << absl::get<std::string>(result);
    }
    array_ = std::move(absl::get<std::unique_ptr<QRCodeArray>>(result));
  }

  std::unique_ptr<QRCodeArray> array_;
};

TEST_F(QRDecodeTest, Metadata) {
  auto result = Decode(*array_);
  ASSERT_FALSE(absl::holds_alternative<std::string>(result))
      << absl::get<std::string>(result);

  std::unique_ptr<QRCode> qrcode =
      std::move(absl::get<std::unique_ptr<QRCode>>(result));

  EXPECT_EQ(29, qrcode->height);
  EXPECT_EQ(29, qrcode->width);
  EXPECT_EQ(3, qrcode->version);
  EXPECT_EQ(5, qrcode->mask_pattern);
  EXPECT_EQ(QRECC_M, qrcode->error_correction);
}

}  // namespace
