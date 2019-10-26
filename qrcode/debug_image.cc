#include "qrcode/debug_image.h"

DebugImage::DebugImage(cv::Mat mat) : mat_(mat) {}

DebugImage DebugImage::FromGray(cv::Mat gray) {
  cv::Mat color;
  cv::cvtColor(gray, color, cv::COLOR_GRAY2BGR);
  return DebugImage(color);
}

void DebugImage::HighlightRow(int row, int from, int to) {
  for (int col = from; col <= to; ++col) {
    uchar* p = mat_.ptr<uchar>(row, col);
    p[0] = 0;
    p[1] = 0;
    p[2] = 255;
  }
}

void DebugImage::HighlightCol(int col, int from, int to) {
  for (int row = from; row <= to; ++row) {
    uchar* p = mat_.ptr<uchar>(row, col);
    p[0] = 0;
    p[1] = 0;
    p[2] = 255;
  }
}

cv::Mat DebugImage::Mat() { return mat_; }
