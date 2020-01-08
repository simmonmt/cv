#ifndef _QRCODE_QR_DECODE_UTILS_H_
#define _QRCODE_QR_DECODE_UTILS_H_ 1

#include "qrcode/qr_array.h"
#include "qrcode/qr_attributes.h"

void UnmaskArray(const QRAttributes& attributes, QRCodeArray* array,
                 unsigned char mask_pattern);

// Find the values of the data blocks in the array. Note that the data blocks
// will be returned the order encountered during array traversal -- from bottom
// to top, from right to left. This isn't the same as the order required for
// decoding.
std::vector<unsigned char> FindDataBlocks(const QRAttributes& attributes,
                                          const QRCodeArray& array);

#endif  // _QRCODE_QR_DECODE_UTILS_H_
