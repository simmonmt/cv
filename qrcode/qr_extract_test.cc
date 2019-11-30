#include "qrcode/qr_extract.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {

constexpr char kTestImageRelPath[] = "qrcode/testdata/straight.png";

TEST(ExtractCode, Test) {
  QRImage qr_image;
  qr_image.image = cv::imread(kTestImageRelPath, cv::IMREAD_GRAYSCALE);
  ASSERT_TRUE(qr_image.image.data != nullptr);

  qr_image.positioning_points.top_left = Point(669, 683);
  qr_image.positioning_points.top_right = Point(1526, 677);
  qr_image.positioning_points.bottom_left = Point(672, 1542);
  qr_image.center = Point(1099, 1110);

  auto result = ExtractCode(qr_image);
  if (absl::holds_alternative<std::string>(result)) {
    ASSERT_TRUE(false) << "extract returned error: "
                       << absl::get<std::string>(result);
  }

  std::unique_ptr<QRCodeArray> array =
      std::move(absl::get<std::unique_ptr<QRCodeArray>>(result));

  array->Dump();

  std::vector<std::string> expected = {
      // 000000001111111111222222222
      // 234567890123456789012345678
      "XXXXXXX   X X XX  X   XXXXXXX",  // 0
      "X     X X X  X  X  X  X     X",  // 1
      "X XXX X     XXX   X X X XXX X",  // 2
      "X XXX X XXX X  XX   X X XXX X",  // 3
      "X XXX X  X    XX   XX X XXX X",  // 4
      "X     X XX XXX XX  X  X     X",  // 5
      "XXXXXXX X X X X X X X XXXXXXX",  // 6
      "         XXX XX    X         ",  // 7
      "XXXXX XXXX X   XX X XX X X X ",  // 8
      " X   X XX X X XXX X XXX  X X ",  // 9
      "XX X  XX  X  X   XXX     XX  ",  // 10
      "X X X       XXX  XXXXXXXXX  X",  // 11
      "XXX X X XXX X  X    XX XX    ",  // 12
      "XXXXXX X X    XX   XX    X X ",  // 13
      "    X X XXXXXX XX  X   X XXXX",  // 14
      "X    X X   X X    XXX XXX  XX",  // 15
      "    XXX  X XXXXX    XX       ",  // 16
      "X X XX XX X X XX   X  X XX X ",  // 17
      "X  XX XX XX XX XX        XX X",  // 18
      "X XXXX  XXX X X    XXXXXX  XX",  // 19
      "X  X  X X X    X    XXXXXXXX ",  // 20
      "        XXX X XX  X X   X  X ",  // 21
      "XXXXXXX X X  X XXXXXX X XXX  ",  // 22
      "X     X   XXX X  XXXX   X XXX",  // 23
      "X XXX X X     XX X XXXXXX XXX",  // 24
      "X XXX X XXX X  X    XX X     ",  // 25
      "X XXX X X  X X XXXX   XX  XX ",  // 26
      "X     X XX XX X    XX    XX X",  // 27
      "XXXXXXX XX  XX X  XXXXX XXX  ",  // 28
  };

  EXPECT_TRUE(array->Get(Point(0, 0)));

  ASSERT_EQ(expected.size(), array->height());
  ASSERT_EQ(expected[0].size(), array->width());
  for (int y = 0; y < expected.size(); ++y) {
    const std::string& row = expected[y];
    for (int x = 0; x < row.size(); ++x) {
      Point p(x, y);
      ASSERT_EQ(row[x] == 'X', array->Get(p)) << p;
    }
  }
}

}  // namespace
