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

// Describes two lines that intersect in the center black area of a
// positioning box.
struct Candidate {
  int h_start_x, h_start_y;
  int lbw;
  int lww;
  int cw;
  int rww;
  int rbw;

  int v_start_x, v_start_y;
  int tbh;
  int twh;
  int ch;
  int bwh;
  int bbh;

  int cross_x, cross_y;

  int center_x, center_y;
};

std::vector<std::pair<int, Candidate>> processRow(
    DebugImage* debug_image, PixelIterator<const uchar>* image_iter, int row) {
  image_iter->SeekRowCol(row, 0);

  // If the row starts with white we need to skip the first set of
  // values returned by the runner.
  bool skip_first = image_iter->Get() != 0;

  Runner runner(image_iter->MakeForwardColumnIterator());
  std::vector<std::pair<int, Candidate>> out;

  if (skip_first) {
    int startx;
    runner.Next(1, &startx);
  }

  for (;;) {
    int startx;
    auto result = runner.Next(5, &startx);
    if (result == absl::nullopt) {
      return out;
    }

    const std::vector<int> lens = std::move(result.value());
    if (IsPositioningBlock(lens)) {
      Candidate cand;
      cand.h_start_x = startx;
      cand.h_start_y = row;
      cand.lbw = lens[0];
      cand.lww = lens[1];
      cand.cw = lens[2];
      cand.rww = lens[3];
      cand.rbw = lens[4];

      int centerx = startx + cand.lbw + cand.lww + cand.cw / 2;
      cand.center_x = centerx;

      image_iter->SeekRowCol(row, centerx);

      auto get_three = [](DirectionalIterator<const uchar> iter) {
        Runner r(iter);
        int startx;
        return r.Next(3, &startx);
      };

      absl::optional<std::vector<int>> maybe_three_up =
          get_three(image_iter->MakeReverseRowIterator());
      absl::optional<std::vector<int>> maybe_three_down =
          get_three(image_iter->MakeForwardRowIterator());

      if (maybe_three_up.has_value() && maybe_three_down.has_value()) {
        const std::vector<int> three_up = std::move(maybe_three_up.value());
        const std::vector<int> three_down = std::move(maybe_three_down.value());

        std::vector<int> combined = {
            three_up[2],                  //
            three_up[1],                  //
            three_up[0] + three_down[0],  //
            three_down[1],                //
            three_down[2],
        };

        if (IsPositioningBlock(combined)) {
          int tot_up = three_up[0] + three_up[1] + three_up[2];

          cand.v_start_x = centerx;
          cand.v_start_y = row - tot_up;
          cand.tbh = combined[0];
          cand.twh = combined[1];
          cand.ch = combined[2];
          cand.bwh = combined[3];
          cand.bbh = combined[4];

          cand.cross_x = centerx;
          cand.cross_y = row;

          cand.center_y = cand.v_start_y + cand.tbh + cand.twh + cand.ch / 2;

          out.emplace_back(std::make_pair(row, cand));
        }
      }
    }

    // The next group starts with white, which is no good to us. Skip it.
    runner.Next(1, &startx);
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
  std::vector<std::pair<int, Candidate>> candidates;
  for (int row = 0; row < image.rows; ++row) {
    if (absl::GetFlag(FLAGS_row) != -1 && absl::GetFlag(FLAGS_row) != row) {
      continue;
    }

    auto row_candidates = processRow(debug_image.get(), &image_iter, row);
    candidates.insert(candidates.end(), row_candidates.begin(),
                      row_candidates.end());
  }

  std::cout << "#candidates found: " << candidates.size() << "\n";

  for (const auto& item : candidates) {
    const Candidate& candidate = item.second;
    debug_image->Crosshairs(candidate.center_y, candidate.center_x);
  }

  if (absl::GetFlag(FLAGS_display)) {
    constexpr char kWindowName[] = "Output";
    cv::namedWindow(kWindowName, cv::WINDOW_NORMAL);
    cv::imshow(kWindowName, debug_image->Mat());
    cv::waitKey(15000);
  }

  return 0;
}
