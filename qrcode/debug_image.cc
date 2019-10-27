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
  if (row < 0 || row >= mat_.rows) {
    std::cerr << "request to highlight invalid row " << row << " valid [0,"
              << mat_.rows - 1 << "]\n";
    return;
  }

  for (int col = from; col <= to; ++col) {
    uchar* p = mat_.ptr<uchar>(row, col);
    p[0] = 0;
    p[1] = 0;
    p[2] = 255;
  }
}

void DebugImage::HighlightCol(int col, int from, int to) {
  if (col < 0 || col >= mat_.cols) {
    std::cerr << "request to highlight invalid col " << col << " valid [0,"
              << mat_.cols - 1 << "]\n";
    return;
  }

  for (int row = from; row <= to; ++row) {
    uchar* p = mat_.ptr<uchar>(row, col);
    p[0] = 0;
    p[1] = 0;
    p[2] = 255;
  }
}

void DebugImage::Crosshairs(const Point& point) {
  int min_y = std::max(point.y - 50, 0);
  int max_y = std::min(point.y + 50, mat_.rows - 1);
  int min_x = std::max(point.x - 50, 0);
  int max_x = std::min(point.x + 50, mat_.cols - 1);

  HighlightRow(point.y - 1, min_x, max_x);
  HighlightRow(point.y, min_x, max_x);
  HighlightRow(point.y + 1, min_x, max_x);
  HighlightCol(point.x - 1, min_y, max_y);
  HighlightCol(point.x, min_y, max_y);
  HighlightCol(point.x + 1, min_y, max_y);
}

cv::Mat DebugImage::Mat() { return mat_; }
