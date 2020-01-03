#ifndef _QRCODE_QR_DECODE_UTILS_H_
#define _QRCODE_QR_DECODE_UTILS_H_ 1

#include "qrcode/qr_array.h"
#include "qrcode/qr_attributes.h"

void UnmaskArray(const QRAttributes& attributes, QRCodeArray* array,
                 unsigned char mask_pattern);

#endif  // _QRCODE_QR_DECODE_UTILS_H_
