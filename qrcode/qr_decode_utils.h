#ifndef _QRCODE_QR_DECODE_UTILS_H_
#define _QRCODE_QR_DECODE_UTILS_H_ 1

#include "qrcode/qr_array.h"
#include "qrcode/qr_attributes.h"
#include "qrcode/qr_error_characteristics_types.h"
#include "qrcode/qr_types.h"

void UnmaskArray(const QRAttributes& attributes, QRCodeArray* array,
                 unsigned char mask_pattern);

// Find the values of the codewords in the array. Note that the codewords
// will be returned the order encountered during array traversal -- from bottom
// to top, from right to left. This isn't the same as the order required for
// decoding.
std::vector<unsigned char> FindCodewords(const QRAttributes& attributes,
                                         const QRCodeArray& array);

std::vector<unsigned char> OrderCodewords(
    const QRErrorLevelCharacteristics& error_characteristics,
    const std::vector<unsigned char>& unordered);

// A single block of codewords from a QR code.
struct CodewordBlock {
  std::vector<unsigned char> data;
  std::vector<unsigned char> ecc;
};

// Given a set of codewords from the array (i.e. as returned by FindCodewords),
// but them back into their blocks.
std::vector<CodewordBlock> SplitCodewordsIntoBlocks(
    const QRErrorLevelCharacteristics& error_characteristics,
    const std::vector<unsigned char>& unordered);

#endif  // _QRCODE_QR_DECODE_UTILS_H_
