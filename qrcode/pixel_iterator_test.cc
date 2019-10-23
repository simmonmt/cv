#include "qrcode/pixel_iterator.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {

using ::testing::ElementsAre;

constexpr unsigned char kData[] = {1,  2,  3,  4,  5,   //
                                   6,  7,  8,  9,  10,  //
                                   11, 12, 13, 14, 15,  //
                                   16, 17, 18, 19, 20};

class PixelIteratorTest : public ::testing::Test {
 public:
  PixelIteratorTest()
      : iter_(kData, 5, 4) {}

  std::vector<int> GetAll(DirectionalIterator<const unsigned char> iter) {
    std::vector<int> out;
    while (iter.Next()) {
      out.push_back(iter.Get());
    }
    return out;
  }

  PixelIterator<const unsigned char> iter_;
};

TEST_F(PixelIteratorTest, Movement) {
  EXPECT_FALSE(iter_.SeekRowCol(-1,0));
  EXPECT_FALSE(iter_.SeekRowCol(0,-1));
  EXPECT_FALSE(iter_.SeekRowCol(-1,-1));

  ASSERT_TRUE(iter_.SeekRowCol(0,0));
  EXPECT_EQ(1, iter_.Get());

  EXPECT_FALSE(iter_.RelSeekRowCol(-1,0));
  EXPECT_FALSE(iter_.RelSeekRowCol(0,-1));
  EXPECT_FALSE(iter_.RelSeekRowCol(-1,-1));
  EXPECT_EQ(1, iter_.Get());

  ASSERT_TRUE(iter_.SeekRowCol(3,4));
  EXPECT_EQ(20, iter_.Get());

  EXPECT_FALSE(iter_.RelSeekRowCol(1,0));
  EXPECT_FALSE(iter_.RelSeekRowCol(0,1));
  EXPECT_FALSE(iter_.RelSeekRowCol(1,1));
  EXPECT_EQ(20, iter_.Get());

  ASSERT_TRUE(iter_.SeekRowCol(2,2));
  EXPECT_EQ(13, iter_.Get());
  ASSERT_TRUE(iter_.RelSeekRowCol(-2,0));
  EXPECT_EQ(3, iter_.Get());
  ASSERT_TRUE(iter_.RelSeekRowCol(3,0));
  EXPECT_EQ(18, iter_.Get());
  ASSERT_TRUE(iter_.RelSeekRowCol(0,-1));
  EXPECT_EQ(17, iter_.Get());
  ASSERT_TRUE(iter_.RelSeekRowCol(0,2));
  EXPECT_EQ(19, iter_.Get());
}

TEST_F(PixelIteratorTest, DirectionalIterator) {
  ASSERT_TRUE(iter_.SeekRowCol(1,1));
  EXPECT_THAT(GetAll(iter_.MakeForwardRowIterator()),
              ElementsAre(12, 17));
  EXPECT_THAT(GetAll(iter_.MakeForwardColumnIterator()),
              ElementsAre(8, 9, 10));

  ASSERT_TRUE(iter_.SeekRowCol(2,2));
  EXPECT_THAT(GetAll(iter_.MakeReverseRowIterator()),
              ElementsAre(8, 3));
  EXPECT_THAT(GetAll(iter_.MakeReverseColumnIterator()),
              ElementsAre(12, 11));
}

}  // namespace
