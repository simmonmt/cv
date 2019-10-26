#ifndef _QRCODE_DEBUG_IMAGE_H_
#define _QRCODE_DEBUG_IMAGE_H_ 1

#include "opencv2/opencv.hpp"

#include "qrcode/point.h"

class DebugImage {
 public:
  DebugImage(cv::Mat mat);
  virtual ~DebugImage() = default;

  static std::unique_ptr<DebugImage> FromGray(cv::Mat gray);

  void HighlightRow(int row, int from, int to);
  void HighlightCol(int col, int from, int to);

  void Crosshairs(const Point& point);

  cv::Mat Mat();

 private:
  DebugImage(const DebugImage&) = default;

  cv::Mat mat_;
};

#endif  // _QRCODE_DEBUG_IMAGE_H_
