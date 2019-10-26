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

void DebugImage::Crosshairs(int row, int col) {
  int min_row = std::max(row - 50, 0);
  int max_row = std::min(row + 50, mat_.rows - 1);
  int min_col = std::max(col - 50, 0);
  int max_col = std::min(col + 50, mat_.cols - 1);

  HighlightRow(row, min_col, max_col);
  HighlightCol(col, min_row, max_row);
}

cv::Mat DebugImage::Mat() { return mat_; }
