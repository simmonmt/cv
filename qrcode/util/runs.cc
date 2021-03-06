// Dumps the the first runs on each row. Useful for diagnosing positioning point
// recognition failures.

#include <iostream>
#include <string>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/strings/str_join.h"
#include "absl/types/optional.h"
#include "opencv2/opencv.hpp"

#include "qrcode/cv_utils.h"
#include "qrcode/pixel_iterator.h"
#include "qrcode/qr_types.h"
#include "qrcode/runner.h"

ABSL_FLAG(std::string, input, "", "input file");
ABSL_FLAG(int, num, 7, "number of runs");

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

  PixelIterator<const uchar> iter = PixelIteratorFromGrayImage(image);
  for (int y = 0; y < image.rows; ++y) {
    std::cout << y << " ";

    iter.Seek(0, y);
    auto result = Runner(iter.MakeForwardColumnIterator())
                      .Next(absl::GetFlag(FLAGS_num), nullptr);
    if (!result.has_value()) {
      std::cout << "-";
    } else {
      std::vector<int> runs = std::move(result.value());
      std::cout << absl::StrJoin(runs, " ");
    }
    std::cout << "\n";
  }

  return 0;
}
