#ifndef _QRCODE_QR_UTILS_H_
#define _QRCODE_QR_UTILS_H_ 1

#include "qrcode/qr_types.h"

// Calculate the center of the code from the positioning points.
Point CalculateCodeCenter(const PositioningPoints& points);

#endif  // _QRCODE_QR_UTILS_H_
