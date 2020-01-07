#include <bitset>
#include <iostream>
#include <memory>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/time/clock.h"
#include "absl/types/variant.h"
#include "opencv2/opencv.hpp"

#include "qrcode/cv_utils.h"
#include "qrcode/point.h"
#include "qrcode/qr_error_characteristics_types.h"
#include "qrcode/qr_extract.h"
#include "qrcode/qr_format.h"
#include "qrcode/qr_locate.h"
#include "qrcode/qr_normalize.h"

ABSL_FLAG(std::string, input, "", "Input file");

struct PointInTime {
  PointInTime(const std::string& name, const absl::Time& time)
      : name(name), time(time) {}

  std::string name;
  absl::Time time;
};

int main(int argc, char** argv) {
  absl::ParseCommandLine(argc, argv);

  if (absl::GetFlag(FLAGS_input).empty()) {
    std::cerr << "--input is required\n";
    return -1;
  }

  std::vector<PointInTime> times = {{"start", absl::Now()}};

  cv::Mat image;
  if (!ReadBwImage(absl::GetFlag(FLAGS_input), image)) {
    std::cerr << "failed to read image\n";
    return -1;
  }

  times.emplace_back("read", absl::Now());

  auto maybe_located_code = LocateCode(image);
  if (absl::holds_alternative<std::string>(maybe_located_code)) {
    std::cerr << "failed to locate code: "
              << absl::get<std::string>(maybe_located_code) << "\n";
    return -1;
  }
  std::unique_ptr<LocatedCode> located_code =
      std::move(absl::get<std::unique_ptr<LocatedCode>>(maybe_located_code));

  times.emplace_back("locate", absl::Now());

  auto maybe_qr_image = NormalizeCode(image, *located_code);
  if (absl::holds_alternative<std::string>(maybe_qr_image)) {
    std::cerr << "failed to normalize code: "
              << absl::get<std::string>(maybe_qr_image) << "\n";
    return -1;
  }
  std::unique_ptr<QRImage> qr_image =
      std::move(absl::get<std::unique_ptr<QRImage>>(maybe_qr_image));

  times.emplace_back("normalize", absl::Now());

  auto maybe_array = ExtractCode(*qr_image);
  if (absl::holds_alternative<std::string>(maybe_array)) {
    std::cerr << "failed to extract code: "
              << absl::get<std::string>(maybe_array) << "\n";
    return -1;
  }
  std::unique_ptr<QRCodeArray> array =
      std::move(absl::get<std::unique_ptr<QRCodeArray>>(maybe_array));

  times.emplace_back("extract", absl::Now());

  for (int y = 0; y < array->height(); ++y) {
    for (int x = 0; x < array->width(); ++x) {
      std::cout << (array->Get(Point(x, y)) ? "X" : " ");
    }
    std::cout << "\n";
  }

  std::cout << "\n";

  auto maybe_format = DecodeFormat(*array);
  if (absl::holds_alternative<std::string>(maybe_format)) {
    std::cerr << "failed to decode format: "
              << absl::get<std::string>(maybe_format) << "\n";
    return -1;
  }
  const QRFormat format = std::move(absl::get<QRFormat>(maybe_format));

  std::cout << "ECC " << format.ecc_level << " mask "
            << std::bitset<3>(format.mask_pattern) << "\n";

  std::cout << "Timing:\n";

  for (int i = 1; i < times.size(); ++i) {
    const PointInTime& ref = times[i - 1];
    const PointInTime& cur = times[i];

    std::cout << "  " << cur.name << ": " << cur.time - ref.time << "\n";
  }

  return 0;
}
