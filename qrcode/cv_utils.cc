#include "qrcode/cv_utils.h"

#include <iostream>

#include "absl/strings/str_format.h"

bool ReadBwImage(const std::string& path, cv::OutputArray out) {
  cv::Mat input = cv::imread(path, cv::IMREAD_COLOR);
  if (!input.data) {
    return false;
  }

  cv::Mat gray;
  cv::cvtColor(input, gray, cv::COLOR_BGR2GRAY);

  // NOTE: This needs to use the same threshold as the threshold()
  // call in ExtractCode, which is awkward.
  cv::threshold(gray, out, 127, 255, cv::THRESH_BINARY);

  if (out.depth() != CV_8U || out.channels() != 1 || !out.isContinuous()) {
    std::cerr << absl::StrFormat(
        "expected depth %d, got %d, chans 1, got %d, "
        "continuous, got %d\n",
        CV_8U, out.depth(), out.channels(), out.isContinuous());
    return false;
  }

  return true;
}
