#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/strings/str_format.h"
#include "absl/strings/str_join.h"
#include "absl/strings/string_view.h"
#include "absl/types/span.h"
#include "opencv2/opencv.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui/highgui.hpp"

#include "qrcode/qr.h"
#include "qrcode/runner.h"

ABSL_FLAG(std::string, input, "", "Input file");
ABSL_FLAG(int, line, -1, "Process this line only");
ABSL_FLAG(bool, display, false, "Display the B&W image");

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

int processRow(absl::Span<const uchar> row) {
  Runner runner(row);

  if (row[0] != 0) {
    // The row starts with white. We need it to start with black. Advance the
    // pointer.
    int startx;
    runner.Next(1, &startx);
  }

  int ret = 0;

  for (;;) {
    int startx;
    auto result = runner.Next(5, &startx);
    if (result == absl::nullopt) {
      return 0;
    }

    const std::vector<int> lens = std::move(result.value());
    if (IsPositioningBlock(lens)) {
      int off = startx;
      for (int i = 0; i < lens.size(); i++) {
	std::cout << absl::StrFormat("S:%d,L:%d ", off, lens[i]);
      }
      std::cout << "\n";
      ret = 1;
    }

    // The next group starts with white, which is no good to us. Skip it.
    runner.Next(1, &startx);
  }

  return ret;
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
  if (absl::GetFlag(FLAGS_line) >= 0) {
    const int row = absl::GetFlag(FLAGS_line);
    if (processRow(absl::Span<const uchar>(p+row*image.cols, image.cols))) {
      std::cout << absl::StrFormat("row %d\n", row);
    }
  } else {
    for (int row = 0; row < image.rows; ++row) {
      if (processRow(absl::Span<const uchar>(p+row*image.cols, image.cols))) {
	std::cout << absl::StrFormat("row %d\n", row);
      }
    }
  }

  if (absl::GetFlag(FLAGS_display)) {
    constexpr char kWindowName[] = "Output";
    cv::namedWindow(kWindowName, 1);
    cv::imshow(kWindowName, image);
    cv::waitKey(0);
  }

  return 0;
}
