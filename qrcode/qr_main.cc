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
#include "qrcode/qr.h"
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

struct Point {
  Point(int x, int y) : x(x), y(y) {}

  int x;
  int y;
};

std::vector<Point> processRow(DebugImage* debug_image,
                              PixelIterator<const uchar>* image_iter, int row) {
  image_iter->SeekRowCol(row, 0);

  // If the row starts with white we need to skip the first set of
  // values returned by the runner.
  bool skip_first = image_iter->Get() != 0;

  Runner runner(image_iter->MakeForwardColumnIterator());
  std::vector<Point> candidates;

  if (skip_first) {
    runner.Next(1, nullptr);
  }

  for (;;) {
    int h_start_x;
    auto result = runner.Next(5, &h_start_x);
    if (result == absl::nullopt) {
      return candidates;
    }

    const std::vector<int> lens = std::move(result.value());
    if (IsPositioningBlock(lens)) {
      const int left_black_width = lens[0];
      const int left_white_width = lens[1];
      const int center_width = lens[2];

      int center_x =
          h_start_x + left_black_width + left_white_width + center_width / 2;

      image_iter->SeekRowCol(row, center_x);

      // {row, center_x} is in the middle of a run of black, in the middle of a
      // series of runs that's compatible with the ratios for a positioning
      // block. If we're truly in the middle of a positioning block, we can
      // confirm that by looking for a series of runs that are compatible with a
      // positioning block on a vertical line that runs through {row, center_x}.
      //
      // Because we know it has to go through {row, center_x}, we can start from
      // that point, looking for black-white-black in either direction. In both
      // cases we'll be looking *out*, and starting from black, so the first
      // black runs we find are actually part of the same run. Join the two
      // together and we can check for positioning ratios.
      //
      // A picture, rotated 90 degrees:
      //
      //     (up)      aaaaa     bbbb+bbbbbbbbbb     ccccc      (down)
      //
      // The horizontal line search got us to +, which is {row, center_x}. Our
      // search up from + finds black run B, then a white run, then black run
      // A. Our search down from + finds the rest of black run B, then a white
      // run, then black run C. Before we check for positioning block ratios we
      // need to combine the results of both searches so we have lengths for
      // black run A, the white run, black run B (which is the sum of the black
      // run Bs from the two searches), the other white run, and black run C. We
      // therefore do a ratio check on this:
      //
      //     (up)      aaaaa     bbbbbbbbbbbbbbb     ccccc      (down)
      //
      // If the ratio check succeeds, center_y is in the middle of run B (which
      // may not be the same as the location of +).
      auto get_three = [](DirectionalIterator<const uchar> iter) {
        return Runner(iter).Next(3, nullptr);
      };

      absl::optional<std::vector<int>> maybe_three_up =
          get_three(image_iter->MakeReverseRowIterator());
      absl::optional<std::vector<int>> maybe_three_down =
          get_three(image_iter->MakeForwardRowIterator());

      if (maybe_three_up.has_value() && maybe_three_down.has_value()) {
        const std::vector<int> three_up = std::move(maybe_three_up.value());
        const std::vector<int> three_down = std::move(maybe_three_down.value());
        const int center_height = three_up[0] + three_down[0];

        std::vector<int> combined = {
            three_up[2],    // top black height
            three_up[1],    // top white height
            center_height,  // center height
            three_down[1],  // bottom white height
            three_down[2],  // bottow black height
        };

        if (IsPositioningBlock(combined)) {
          const int center_y = row - three_up[0] + center_height / 2;
          candidates.emplace_back(center_x, center_y);
        }
      }
    }

    // The next group starts with white, which is no good to us. Skip it.
    runner.Next(1, nullptr);
  }
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

  std::unique_ptr<DebugImage> debug_image = DebugImage::FromGray(image);

  if (image.depth() != CV_8U || image.channels() != 1 ||
      !image.isContinuous()) {
    std::cerr << absl::StrFormat(
        "expected depth %d, got %d, chans 1, got %d, "
        "continuous, got %d\n",
        CV_8U, image.depth(), image.channels(), image.isContinuous());
    return -1;
  }

  PixelIterator<const uchar> image_iter(image.ptr<uchar>(0), image.cols,
                                        image.rows);
  std::vector<Point> candidates;
  for (int row = 0; row < image.rows; ++row) {
    if (absl::GetFlag(FLAGS_row) != -1 && absl::GetFlag(FLAGS_row) != row) {
      continue;
    }

    auto row_candidates = processRow(debug_image.get(), &image_iter, row);
    candidates.insert(candidates.end(), row_candidates.begin(),
                      row_candidates.end());
  }

  std::cout << "#candidates found: " << candidates.size() << "\n";

  for (const Point& candidate : candidates) {
    debug_image->Crosshairs(candidate.y, candidate.x);
  }

  if (absl::GetFlag(FLAGS_display)) {
    constexpr char kWindowName[] = "Output";
    cv::namedWindow(kWindowName, cv::WINDOW_NORMAL);
    cv::imshow(kWindowName, debug_image->Mat());
    cv::waitKey(15000);
  }

  return 0;
}
