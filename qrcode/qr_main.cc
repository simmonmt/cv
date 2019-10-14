#include "absl/strings/str_format.h"
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

  cv::Mat image;
  if (readBwImage(FLAGS_input, image) < 0) {
    std::cerr << "failed to read image\n";
    return -1;
  }

  if (image.depth() != CV_8U || image.channels() != 1 ||
      !image.isContinuous()) {
    std::cerr << absl::StrFormat("expected depth %d, got %d, chans 1, got %d, "
				 "continuous, got %d\n",
				 CV_8U, image.depth(), image.channels(),
				 image.isContinuous());
    return -1;
  }

  uchar* p = image.ptr<uchar>(0);
  for (int row = 0; row < image.rows; ++row) {
    int num = 0;
    for (int col = 0; col < image.cols; ++col) {
      num += p[row*image.cols + col] == 0;
    }
    std::cout << absl::StrFormat("row %d: %d\n", row, num);
  }

  // constexpr char kWindowName[] = "Output";
  // cv::namedWindow(kWindowName, 1);
  // cv::imshow(kWindowName, image);
  // cv::waitKey(0);

  return 0;
}
