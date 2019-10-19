#include "qrcode/pixel_iterator.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {

constexpr unsigned char kData[] = {1,  2,  3,  4,  5,   //
                                   6,  7,  8,  9,  10,  //
                                   11, 12, 13, 14, 15,  //
                                   16, 17, 18, 19, 20};

class PixelIteratorTest : public ::testing::Test {};

TEST_F(PixelIteratorTest, Simple) {
  PixelIterator<const unsigned char> iter(kData, 5, 4);

  ASSERT_EQ(1, iter.Get());
  ASSERT_TRUE(iter.NextCol());
  ASSERT_EQ(2, iter.Get());
  ASSERT_TRUE(iter.NextRow());
  ASSERT_EQ(7, iter.Get());

  ASSERT_FALSE(iter.SeekRow(-1));
  ASSERT_FALSE(iter.SeekRow(4));
  ASSERT_FALSE(iter.SeekCol(-1));
  ASSERT_FALSE(iter.SeekCol(5));

  ASSERT_TRUE(iter.SeekRow(3));
  ASSERT_TRUE(iter.SeekCol(4));
  ASSERT_EQ(20, iter.Get());
  ASSERT_FALSE(iter.NextCol());
  ASSERT_EQ(20, iter.Get());
  ASSERT_FALSE(iter.NextRow());
  ASSERT_EQ(20, iter.Get());

  ASSERT_TRUE(iter.SeekRow(1));
  ASSERT_TRUE(iter.SeekCol(2));
  ASSERT_EQ(8, iter.Get());
}

}  // namespace
