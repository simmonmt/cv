// Dumps the the first runs on each row. Useful for diagnosing positioning point
// recognition failures.

#include <iostream>
#include <string>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/strings/str_join.h"
#include "absl/types/optional.h"
#include "opencv2/opencv.hpp"

#include "qrcode/pixel_iterator.h"
#include "qrcode/runner.h"

ABSL_FLAG(std::string, input, "", "input file");
ABSL_FLAG(int, num, 7, "number of runs");

int main(int argc, char** argv) {
  absl::ParseCommandLine(argc, argv);

  if (absl::GetFlag(FLAGS_input).empty()) {
    std::cerr << "--input is required\n";
    return -1;
  }

  cv::Mat image = cv::imread(absl::GetFlag(FLAGS_input), cv::IMREAD_GRAYSCALE);
  if (image.data == nullptr) {
    std::cerr << "failed to read image\n";
    return -1;
  }

  PixelIterator<const uchar> iter(image.ptr<uchar>(0), image.cols, image.rows);
  for (int row = 0; row < image.rows; ++row) {
    std::cout << row << " ";

    iter.SeekRowCol(row, 0);
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
