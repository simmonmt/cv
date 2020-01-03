#include "qrcode/qr_array.h"

#include "gtest/gtest.h"

namespace {

TEST(QRCodeArrayTest, Test) {
  QRCodeArray arr(3, 5);
  arr.Set(Point(0, 0), true);
  EXPECT_TRUE(arr.Get(Point(0, 0)));
  EXPECT_FALSE(arr.Get(Point(1, 0)));
}

}  // namespace
