#include "gflags/gflags.h"
#include "opencv2/opencv.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui/highgui.hpp"

DEFINE_string(input, "", "Input file");

static bool IsNonEmptyMessage(const char* flagname, const std::string& value) {
  return !value.empty();
}
DEFINE_validator(input, &IsNonEmptyMessage);

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  constexpr char kWindowName[] = "Output";

  cv::Mat input = cv::imread(FLAGS_input, cv::IMREAD_COLOR);
  if (!input.data) {
    std::cerr << "failed to read image\n";
    return -1;
  }

  cv::Mat gray;
  cv::cvtColor(input, gray, cv::COLOR_BGR2GRAY);

  cv::Mat bw;
  cv::threshold(gray, bw, 127, 255, cv::THRESH_BINARY);

  cv::namedWindow(kWindowName, 1);
  cv::imshow(kWindowName, bw);
  cv::waitKey(0);

  return 0;
}
