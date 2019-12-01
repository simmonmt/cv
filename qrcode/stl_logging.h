#ifndef _QRCODE_STL_LOGGING_H_
#define _QRCODE_STL_LOGGING_H_ 1

#include <iostream>
#include <vector>

std::ostream& operator<<(std::ostream& str, const std::vector<bool>& vec);

#endif  // _QRCODE_STL_LOGGING_H_
