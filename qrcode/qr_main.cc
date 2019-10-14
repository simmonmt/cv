#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/strings/str_format.h"
#include "absl/strings/string_view.h"
#include "absl/types/span.h"
#include "opencv2/opencv.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui/highgui.hpp"

ABSL_FLAG(std::string, input, "", "Input file");

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

int processRow(absl::Span<uchar> row) {
  int num = 0;
  for (const auto p : row) {
    num += p == 0;
  }
  return num;
}

}  // namespace

int main(int argc, char** argv) {
  absl::ParseCommandLine(argc, argv);

  if (absl::GetFlag(FLAGS_input).empty()) {
    std::cerr << "--input is required\n";
    return -1;
  }

  cv::Mat image;
  if (readBwImage(absl::GetFlag(FLAGS_input), image) < 0) {
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
    int num = processRow(absl::Span<uchar>(p+row*image.cols, image.cols));
    std::cout << absl::StrFormat("row %d: %d\n", row, num);
  }

  // constexpr char kWindowName[] = "Output";
  // cv::namedWindow(kWindowName, 1);
  // cv::imshow(kWindowName, image);
  // cv::waitKey(0);

  return 0;
}
