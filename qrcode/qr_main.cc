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

#include "qrcode/cv_utils.h"
#include "qrcode/debug_image.h"
#include "qrcode/point.h"
#include "qrcode/qr_locate.h"
#include "qrcode/qr_normalize.h"
#include "qrcode/qr_types.h"
#include "qrcode/runner.h"

ABSL_FLAG(std::string, input, "", "Input file");
ABSL_FLAG(bool, display, false, "Display the B&W image");
ABSL_FLAG(int, row, -1, "Use this row only for the first scan");

int main(int argc, char** argv) {
  absl::ParseCommandLine(argc, argv);

  if (absl::GetFlag(FLAGS_input).empty()) {
    std::cerr << "--input is required\n";
    return -1;
  }

  cv::Mat image;
  if (!ReadBwImage(absl::GetFlag(FLAGS_input), image)) {
    std::cerr << "failed to read image\n";
    return -1;
  }

  std::unique_ptr<DebugImage> debug_image = DebugImage::FromGray(image);

  auto maybe_located_code = LocateCode(image);
  if (absl::holds_alternative<std::string>(maybe_located_code)) {
    std::cerr << "failed to locate code: "
              << absl::get<std::string>(maybe_located_code);
    return -1;
  }

  std::unique_ptr<LocatedCode> located_code =
      std::move(absl::get<std::unique_ptr<LocatedCode>>(maybe_located_code));

  auto maybe_qr_image = NormalizeCode(image, *located_code);
  if (absl::holds_alternative<std::string>(maybe_qr_image)) {
    std::cerr << "failed to extract code: "
              << absl::get<std::string>(maybe_qr_image);
    return -1;
  }

  std::unique_ptr<QRImage> qr_image =
      std::move(absl::get<std::unique_ptr<QRImage>>(maybe_qr_image));

  if (absl::GetFlag(FLAGS_display)) {
    constexpr char kWindowName[] = "Output";
    cv::namedWindow(kWindowName, cv::WINDOW_NORMAL);
    // cv::imshow(kWindowName, debug_image->Mat());
    cv::imshow(kWindowName, qr_image->image);
    cv::waitKey(0);
  }

  return 0;
}
