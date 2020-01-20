#include "qrcode/gf_mat.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {

using ::testing::ElementsAre;
using ::testing::Eq;
using ::testing::IsNull;

class GFMatTest : public ::testing::Test {
 public:
  GF16 gf_;
};

TEST_F(GFMatTest, Row) {
  GFMat mat(gf_, 2, 3);
  mat.Set(0, 0, 10);
  mat.Set(0, 1, 20);
  mat.Set(1, 0, 30);

  EXPECT_THAT(mat.Row(0), ElementsAre(10, 20, 0));
  EXPECT_THAT(mat.Row(1), ElementsAre(30, 0, 0));
}

TEST_F(GFMatTest, Mult_BadDims) {
  EXPECT_THAT(GFMat(gf_, 3, 1).Mult(GFMat(gf_, 2, 3)), IsNull());
}

TEST_F(GFMatTest, Mult_1x3_3x1) {
  GFMat a(gf_, 1, 3);
  a.Set(0, 0, 0b0001);
  a.Set(0, 1, 0b0010);
  a.Set(0, 2, 0b0011);

  GFMat b(gf_, 3, 1);
  b.Set(0, 0, 0b1001);
  b.Set(1, 0, 0b1010);
  b.Set(2, 0, 0b1011);

  std::unique_ptr<GFMat> c = a.Mult(b);
  ASSERT_EQ(1, c->h());
  ASSERT_EQ(1, c->w());

  EXPECT_EQ(gf_.Add({gf_.Mult(0b0001, 0b1001), gf_.Mult(0b0010, 0b1010),
                     gf_.Mult(0b0011, 0b1011)}),
            c->Get(0, 0));
}

TEST_F(GFMatTest, Mult_3x1_1x3) {
  GFMat a(gf_, 3, 1);
  a.Set(0, 0, 0b1001);
  a.Set(1, 0, 0b1010);
  a.Set(2, 0, 0b1011);

  GFMat b(gf_, 1, 3);
  b.Set(0, 0, 0b0001);
  b.Set(0, 1, 0b0010);
  b.Set(0, 2, 0b0011);

  std::unique_ptr<GFMat> c = a.Mult(b);
  ASSERT_EQ(3, c->h());
  ASSERT_EQ(3, c->w());

  EXPECT_THAT(c->Row(0), ElementsAre(gf_.Mult(0b1001, 0b0001),  //
                                     gf_.Mult(0b1001, 0b0010),  //
                                     gf_.Mult(0b1001, 0b0011)));
  EXPECT_THAT(c->Row(1), ElementsAre(gf_.Mult(0b1010, 0b0001),  //
                                     gf_.Mult(0b1010, 0b0010),  //
                                     gf_.Mult(0b1010, 0b0011)));
  EXPECT_THAT(c->Row(2), ElementsAre(gf_.Mult(0b1011, 0b0001),  //
                                     gf_.Mult(0b1011, 0b0010),  //
                                     gf_.Mult(0b1011, 0b0011)));
}

class GFSqMatTest : public GFMatTest {};

TEST_F(GFSqMatTest, Determinant2x2) {
  GFSqMat mat(gf_, 2);
  mat.Set(0, 0, 0b0001);
  mat.Set(0, 1, 0b0010);
  mat.Set(1, 0, 0b0100);
  mat.Set(1, 1, 0b1000);

  EXPECT_THAT(mat.Determinant(),
              Eq(gf_.Add({0b1000, gf_.Mult(0b0010, 0b0100)})));
}

TEST_F(GFSqMatTest, Determinant3x3) {
  GFSqMat mat(gf_, 3);
  mat.Set(0, 0, 0b0001);
  mat.Set(0, 1, 0b0010);
  mat.Set(0, 2, 0b0011);
  mat.Set(1, 0, 0b0101);
  mat.Set(1, 1, 0b0110);
  mat.Set(1, 2, 0b0111);
  mat.Set(2, 0, 0b1001);
  mat.Set(2, 1, 0b1010);
  mat.Set(2, 2, 0b1011);

  EXPECT_THAT(mat.Determinant(), Eq(gf_.Add({0b1100, 0b0101, 0b1001})));
}

bool IsIdentityMatrix(const GFMat& mat) {
  if (mat.w() != mat.h()) {
    return false;
  }

  for (int row = 1; row < mat.h(); ++row) {
    for (int col = 0; col < mat.w(); ++col) {
      unsigned char want = 0;
      if (row == col) {
        want = 1;
      }
      if (mat.Get(row, col) != want) {
        return false;
      }
    }
  }
  return true;
}

TEST_F(GFSqMatTest, Inverse1x1) {
  GFSqMat mat(gf_, 1);
  mat.Set(0, 0, 0b1011);

  std::unique_ptr<GFSqMat> inv = mat.Inverse();
  std::unique_ptr<GFMat> res = mat.Mult(*inv);
  EXPECT_TRUE(IsIdentityMatrix(*res));
}

TEST_F(GFSqMatTest, Inverse2x2) {
  GFSqMat mat(gf_, 2);
  mat.Set(0, 0, 0b1011);
  mat.Set(0, 1, 0b1001);
  mat.Set(1, 0, 0b1001);
  mat.Set(1, 1, 0b1011);

  std::unique_ptr<GFSqMat> inv = mat.Inverse();
  std::unique_ptr<GFMat> res = mat.Mult(*inv);
  EXPECT_TRUE(IsIdentityMatrix(*res));
}

}  // namespace
