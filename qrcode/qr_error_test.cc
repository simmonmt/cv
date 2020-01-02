#include "qrcode/qr_error.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {

using ::testing::HasSubstr;
using ::testing::VariantWith;

TEST(QRErrorTest, GetErrorCharacteristics) {
  EXPECT_THAT(GetErrorCharacteristics(99, QRECC_L),
              VariantWith<std::string>(HasSubstr("version 99 level QRECC_L")));
}

}  // namespace
