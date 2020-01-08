#ifndef _QRCODE_QR_ERROR_CHARACTERISTICS_TYPES_H_
#define _QRCODE_QR_ERROR_CHARACTERISTICS_TYPES_H_ 1

#include <iostream>
#include <vector>

enum QRErrorCorrection {
  QRECC_L,
  QRECC_M,
  QRECC_Q,
  QRECC_H,
};

// Describes the ECC characteristics for a given version and ECC level.
struct QRErrorLevelCharacteristics {
  int total_data_codewords;
  int total_ecc_codewords;

  struct BlockSet {
    int num_blocks;
    int block_codewords;
    int data_codewords;
  };

  std::vector<BlockSet> block_sets;
};

std::ostream& operator<<(std::ostream& str, const QRErrorCorrection level);

#endif  // _QRCODE_QR_ERROR_CHARACTERISTICS_TYPES_H_
