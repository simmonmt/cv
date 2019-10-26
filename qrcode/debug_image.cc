#include "qrcode/debug_image.h"

DebugImage::DebugImage(cv::Mat image) : image_(image) {}

void DebugImage::HighlightLine(cv::Point2i from, cv::Point2i to) {}

cv::Mat DebugImage::Image() { return image_; }
