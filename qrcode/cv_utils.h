#ifndef _QRCODE_CV_UTILS_H_
#define _QRCODE_CV_UTILS_H_ 1

#include <string>

#include "opencv2/opencv.hpp"

bool ReadBwImage(const std::string& path, cv::OutputArray out);

#endif  // _QRCODE_CV_UTILS_H_
