#include "qrcode/debug_image.h"

#include <memory>

#include "absl/memory/memory.h"

DebugImage::DebugImage(cv::Mat mat) : mat_(mat) {}

std::unique_ptr<DebugImage> DebugImage::FromGray(cv::Mat gray) {
  cv::Mat color;
  cv::cvtColor(gray, color, cv::COLOR_GRAY2BGR);
  return absl::make_unique<DebugImage>(color);
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
