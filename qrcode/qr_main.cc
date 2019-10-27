#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/memory/memory.h"
#include "absl/strings/str_format.h"
#include "absl/strings/str_join.h"
#include "absl/strings/string_view.h"
#include "absl/types/span.h"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/opencv.hpp"

#include "qrcode/debug_image.h"
#include "qrcode/point.h"
#include "qrcode/qr_locate.h"
#include "qrcode/qr_types.h"
#include "qrcode/runner.h"

ABSL_FLAG(std::string, input, "", "Input file");
ABSL_FLAG(bool, display, false, "Display the B&W image");
ABSL_FLAG(int, row, -1, "Use this row only for the first scan");

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
    std::cerr << absl::StrFormat(
        "expected depth %d, got %d, chans 1, got %d, "
        "continuous, got %d\n",
        CV_8U, image.depth(), image.channels(), image.isContinuous());
    return -1;
  }

  std::unique_ptr<DebugImage> debug_image = DebugImage::FromGray(image);

  auto maybe_located_code = LocateCode(image);
  if (absl::holds_alternative<std::string>(maybe_located_code)) {
    std::cerr << "failed to located code: "
              << absl::get<std::string>(maybe_located_code);
    return -1;
  }

  std::unique_ptr<LocatedCode> located_code =
      std::move(absl::get<std::unique_ptr<LocatedCode>>(maybe_located_code));

  cv::Mat rotation_matrix = cv::getRotationMatrix2D(
      cv::Point2f(located_code->center.x, located_code->center.y),
      -located_code->rotation_angle, 1.0);

  cv::Mat rotated_image;
  cv::warpAffine(image, rotated_image, rotation_matrix,
                 {image.cols, image.rows});

  if (absl::GetFlag(FLAGS_display)) {
    constexpr char kWindowName[] = "Output";
    cv::namedWindow(kWindowName, cv::WINDOW_NORMAL);
    // cv::imshow(kWindowName, debug_image->Mat());
    cv::imshow(kWindowName, rotated_image);
    cv::waitKey(0);
  }

  return 0;
}
