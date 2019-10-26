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

std::vector<Candidate> processRow(DirectionalIterator<const uchar> iter) {
  Runner runner(iter);
  std::vector<Candidate> out;

  if (iter.Get() != 0) {
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
    image_iter.SeekRowCol(row, 0);
    std::vector<Candidate> row_candidates =
        processRow(image_iter.MakeForwardColumnIterator());
    if (!row_candidates.empty()) {
      std::cout << "row " << row << ": #candidates " << row_candidates.size()
                << "\n";
      for (const auto& candidate : row_candidates) {
        candidates.emplace_back(row, candidate);
      }
    }
  }

  std::cout << "#candidates found: " << candidates.size() << "\n";

  for (const auto& item : candidates) {
    const int row = item.first;
    const Candidate& candidate = item.second;
    const int tot_len =
        candidate.lb + candidate.lw + candidate.c + candidate.rw + candidate.rb;
    debug_image->HighlightRow(row, candidate.start, candidate.start + tot_len);
  }

  if (absl::GetFlag(FLAGS_display)) {
    constexpr char kWindowName[] = "Output";
    cv::namedWindow(kWindowName, cv::WINDOW_NORMAL);
    cv::imshow(kWindowName, debug_image->Mat());
    cv::waitKey(5000);
  }

  return 0;
}
