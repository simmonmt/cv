#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/strings/str_format.h"
#include "absl/strings/str_join.h"
#include "absl/strings/string_view.h"
#include "absl/types/span.h"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/opencv.hpp"
#include "qrcode/qr.h"
#include "qrcode/runner.h"

ABSL_FLAG(std::string, input, "", "Input file");
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

struct Candidate {
  Candidate(int start, int lb, int lw, int c, int rw, int rb)
      : start(start), lb(lb), lw(lw), c(c), rw(rw), rb(rb) {}

  int start, lb, lw, c, rw, rb;
};

std::vector<Candidate> processRow(absl::Span<const uchar> row) {
  Runner runner(row);
  std::vector<Candidate> out;

  if (row[0] != 0) {
    // The row starts with white. We need it to start with black. Advance the
    // pointer.
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
      out.emplace_back(startx, lens[0], lens[1], lens[2], lens[3], lens[4]);
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

  if (image.depth() != CV_8U || image.channels() != 1 ||
      !image.isContinuous()) {
    std::cerr << absl::StrFormat(
        "expected depth %d, got %d, chans 1, got %d, "
        "continuous, got %d\n",
        CV_8U, image.depth(), image.channels(), image.isContinuous());
    return -1;
  }

  uchar* image_px = image.ptr<uchar>(0);
  std::vector<std::pair<int, Candidate>> candidates;
  for (int row = 0; row < image.rows; ++row) {
    std::vector<Candidate> row_candidates = processRow(
        absl::Span<const uchar>(image_px + row * image.cols, image.cols));
    for (const auto& candidate : row_candidates) {
      candidates.emplace_back(row, candidate);
    }
  }

  cv::Mat annotated;
  cv::cvtColor(image, annotated, cv::COLOR_GRAY2BGR);
  uchar* annotated_px = annotated.ptr<uchar>(0);

  for (const auto& item : candidates) {
    const int row = item.first;
    const Candidate& candidate = item.second;
    const int tot_len =
        candidate.lb + candidate.lw + candidate.c + candidate.rw + candidate.rb;

    for (int i = 0; i < tot_len; ++i) {
      int px_idx = row * image.cols + candidate.start + i;
      if (image_px[px_idx]) {
        annotated_px[px_idx * 3] = 127;
      } else {
        annotated_px[px_idx * 3] = 255;
      }
    }
  }

  if (absl::GetFlag(FLAGS_display)) {
    constexpr char kWindowName[] = "Output";
    cv::namedWindow(kWindowName, 1);
    cv::imshow(kWindowName, annotated);
    cv::waitKey(0);
  }

  return 0;
}
