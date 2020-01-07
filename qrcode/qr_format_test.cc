#include "qrcode/qr_format.h"

#include "gtest/gtest.h"

#include "qrcode/testutils.h"

namespace {

constexpr char kTestDataSpecExamplePath[] =
    "qrcode/testdata/spec_example_1m.txt";

class DecodeFormatTest : public ::testing::Test {
 public:
  void SetUp() override {
    ASSIGN_OR_ASSERT(base_, ReadQRCodeArrayFromFile(kTestDataSpecExamplePath),
                     "read failed");
  }

  std::unique_ptr<QRCodeArray> base_;
};

TEST_F(DecodeFormatTest, Test) {
  ASSIGN_OR_ASSERT(QRFormat format, DecodeFormat(*base_), "decode failed");

  // The spec says it's mask pattern 010, but it's actually 011 because we can't
  // have nice things.
  EXPECT_EQ(QRECC_M, format.ecc_level);
  EXPECT_EQ(0b011, format.mask_pattern);
}

}  // namespace
