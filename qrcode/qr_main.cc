#include "gflags/gflags.h"
#include "opencv2/opencv.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui/highgui.hpp"

DEFINE_string(input, "", "Input file");

static bool IsNonEmptyMessage(const char* flagname, const std::string& value) {
  return !value.empty();
}
DEFINE_validator(input, &IsNonEmptyMessage);

namespace {

int readBwImage(const std::string& path, cv::OutputArray out) {
  cv::Mat input = cv::imread(path, cv::IMREAD_COLOR);
  if (!input.data) {
    return -1;
  }

  cv::Mat gray;
  cv::cvtColor(input, gray, cv::COLOR_BGR2GRAY);
  cv::threshold(gray, out, 127, 255, cv::THRESH_BINARY);

  return 0;
}

}  // namespace

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  constexpr char kWindowName[] = "Output";

  cv::Mat image;
  if (readBwImage(FLAGS_input, image) < 0) {
    std::cerr << "failed to read image\n";
    return -1;
  }

  cv::namedWindow(kWindowName, 1);
  cv::imshow(kWindowName, image);
  cv::waitKey(0);

  return 0;
}
