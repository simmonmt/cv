#ifndef _QRCODE_DEBUG_IMAGE_H_
#define _QRCODE_DEBUG_IMAGE_H_ 1

#include "opencv2/opencv.hpp"

class DebugImage {
 public:
  DebugImage(cv::Mat image);
  virtual ~DebugImage() = default;

  DebugImage(const DebugImage&) = delete;

  void HighlightLine(cv::Point2i from, cv::Point2i to);

  cv::Mat Image();

 private:
  cv::Mat image_;
};

#endif  // _QRCODE_DEBUG_IMAGE_H_
