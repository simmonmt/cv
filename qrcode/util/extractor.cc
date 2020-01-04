#include <iostream>
#include <memory>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/types/variant.h"
#include "opencv2/opencv.hpp"

#include "qrcode/cv_utils.h"
#include "qrcode/point.h"
#include "qrcode/qr_extract.h"
#include "qrcode/qr_locate.h"
#include "qrcode/qr_normalize.h"

ABSL_FLAG(std::string, input, "", "Input file");

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

  auto maybe_located_code = LocateCode(image);
  if (absl::holds_alternative<std::string>(maybe_located_code)) {
    std::cerr << "failed to locate code: "
              << absl::get<std::string>(maybe_located_code) << "\n";
    return -1;
  }
  std::unique_ptr<LocatedCode> located_code =
      std::move(absl::get<std::unique_ptr<LocatedCode>>(maybe_located_code));

  auto maybe_qr_image = NormalizeCode(image, *located_code);
  if (absl::holds_alternative<std::string>(maybe_qr_image)) {
    std::cerr << "failed to normalize code: "
              << absl::get<std::string>(maybe_qr_image) << "\n";
    return -1;
  }
  std::unique_ptr<QRImage> qr_image =
      std::move(absl::get<std::unique_ptr<QRImage>>(maybe_qr_image));

  auto maybe_array = ExtractCode(*qr_image);
  if (absl::holds_alternative<std::string>(maybe_array)) {
    std::cerr << "failed to extract code: "
              << absl::get<std::string>(maybe_array) << "\n";
    return -1;
  }
  std::unique_ptr<QRCodeArray> array =
      std::move(absl::get<std::unique_ptr<QRCodeArray>>(maybe_array));

  for (int y = 0; y < array->height(); ++y) {
    for (int x = 0; x < array->width(); ++x) {
      std::cout << (array->Get(Point(x, y)) ? "X" : " ");
    }
    std::cout << "\n";
  }

  return 0;
}
