#include "qrcode/qr_extract_utils.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "qrcode/pixel_iterator.h"
#include "qrcode/qr_types.h"
#include "qrcode/testutils.h"

namespace {

using ::testing::Eq;

TEST(RecenterTest, Test) {
  // Use runs of odd length, which means 'center' is well defined.
  std::vector<unsigned char> data = MakeRun({
      //                  0 1 2 3 4 5 6 7
      7, 1,        //  0: 1 1 1 1 1 1 1 0
      1, 5, 1, 1,  //  1: 1 0 0 0 0 0 1 0
      1, 5, 1, 1,  //  2: 1 0 0 0 0 0 1 0
      1, 5, 1, 1,  //  3: 1 0 0 0 0 0 1 0
      1, 5, 1, 1,  //  4: 1 0 0 0 0 0 1 0
      1, 5, 1, 1,  //  5: 1 0 0 0 0 0 1 0
      7, 1,        //  6: 1 1 1 1 1 1 1 0
  });
  PixelIterator<const unsigned char> iter(data.data(), 8, 7);

  EXPECT_THAT(RecenterPositioningPoint(Point(2, 2), iter),
              Eq(Point(3, 3)));  // center
  EXPECT_THAT(RecenterPositioningPoint(Point(3, 3), iter),
              Eq(Point(3, 3)));  // slightly off
  EXPECT_THAT(RecenterPositioningPoint(Point(4, 4), iter),
              Eq(Point(3, 3)));  // slightly off
  EXPECT_THAT(RecenterPositioningPoint(Point(2, 5), iter),
              Eq(Point(3, 3)));  // way off
  EXPECT_THAT(RecenterPositioningPoint(Point(5, 1), iter),
              Eq(Point(3, 3)));  // way off

  // Use runs of even length, which means 'center' is ambiguous.
  data = MakeRun({
      //                  0 1 2 3 4 5 6 7 8
      8, 1,        //  0: 1 1 1 1 1 1 1 1 0
      1, 6, 1, 1,  //  1: 1 0 0 0 0 0 0 1 0
      1, 6, 1, 1,  //  2: 1 0 0 0 0 0 0 1 0
      1, 6, 1, 1,  //  3: 1 0 0 0 0 0 0 1 0
      1, 6, 1, 1,  //  4: 1 0 0 0 0 0 0 1 0
      1, 6, 1, 1,  //  5: 1 0 0 0 0 0 0 1 0
      1, 6, 1, 1,  //  6: 1 0 0 0 0 0 0 1 0
      8, 1,        //  7: 1 1 1 1 1 1 1 1 0
  });
  iter = PixelIterator<const unsigned char>(data.data(), 9, 8);

  EXPECT_THAT(RecenterPositioningPoint(Point(2, 2), iter),
              Eq(Point(3, 3)));  // slightly off
  EXPECT_THAT(RecenterPositioningPoint(Point(3, 3), iter),
              Eq(Point(3, 3)));  // center
  EXPECT_THAT(RecenterPositioningPoint(Point(4, 4), iter),
              Eq(Point(4, 4)));  // center
  EXPECT_THAT(RecenterPositioningPoint(Point(5, 5), iter),
              Eq(Point(4, 4)));  // slightly off
  EXPECT_THAT(RecenterPositioningPoint(Point(2, 6), iter),
              Eq(Point(3, 4)));  // way off
  EXPECT_THAT(RecenterPositioningPoint(Point(5, 1), iter),
              Eq(Point(4, 3)));  // way off
}

}  // namespace
