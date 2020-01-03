#include "qrcode/qr_error_characteristics.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {

using ::testing::ElementsAre;
using ::testing::Field;
using ::testing::HasSubstr;
using ::testing::VariantWith;

MATCHER_P(BlockSetEq, bs, "") {
  return bs.num_blocks == arg.num_blocks &&
         bs.total_codewords == arg.total_codewords &&
         bs.data_codewords == arg.data_codewords;
}

QRErrorLevelCharacteristics::BlockSet MakeBlockSet(int num_blocks,
                                                   int total_codewords,
                                                   int data_codewords) {
  return {num_blocks, total_codewords, data_codewords};
}

TEST(QRErrorTest, GetErrorCharacteristics) {
  EXPECT_THAT(GetErrorCharacteristics(5, QRECC_Q),
              VariantWith<QRErrorLevelCharacteristics>(
                  Field(&QRErrorLevelCharacteristics::block_sets,
                        ElementsAre(BlockSetEq(MakeBlockSet(2, 33, 15)),
                                    BlockSetEq(MakeBlockSet(2, 34, 16))))));

  EXPECT_THAT(GetErrorCharacteristics(99, QRECC_L),
              VariantWith<std::string>(HasSubstr("version 99 level QRECC_L")));
}

}  // namespace
