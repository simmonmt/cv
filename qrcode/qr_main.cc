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

  cv::namedWindow(kWindowName, 1);
  cv::imshow(kWindowName, input);
  cv::waitKey(0);

  return 0;
}
