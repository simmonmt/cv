#include "qrcode/qr_decode.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {

TEST(QRDecodeTest, Test) {
  QRCodeArray array(1, 1);
  auto result = Decode(array);
  if (absl::holds_alternative<std::string>(result)) {
    ASSERT_TRUE(false) << "decode returned error: "
                       << absl::get<std::string>(result);
  }

  std::unique_ptr<QRCode> qrcode =
      std::move(absl::get<std::unique_ptr<QRCode>>(result));
}

}  // namespace
