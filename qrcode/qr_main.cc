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

struct Candidate {
  Candidate(int start, int lb, int lw, int c, int rw, int rb)
      : start(start), lb(lb), lw(lw), c(c), rw(rw), rb(rb) {}

  int start, lb, lw, c, rw, rb;
};

std::ostream& operator<<(std::ostream& stream, const Candidate& cand) {
  return stream << absl::StrFormat("cand<%d:%d %d %d %d %d>", cand.start,
                                   cand.lb, cand.lw, cand.c, cand.rw, cand.rb);
}

template <class T>
std::ostream& operator<<(std::ostream& stream, const std::vector<T>& vec) {
  for (int i = 0; i < vec.size(); ++i) {
    if (i != 0) {
      stream << " ";
    }
    stream << absl::StrCat(vec[i]);
  }
  return stream;
}

std::vector<std::pair<int, Candidate>> processRow(
    PixelIterator<const uchar>* image_iter, int row) {
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
      Candidate cand(startx, lens[0], lens[1], lens[2], lens[3], lens[4]);

      std::cout << cand << "\n";

      int centerx = startx + cand.lb + cand.lw + cand.c / 2;

      std::cout << "looking at col " << centerx << " row " << row << "\n";

      image_iter->SeekRowCol(row, centerx);

      auto get_three = [&](DirectionalIterator<const uchar> iter) {
        Runner r(iter);
        int startx;
        return runner.Next(3, &startx);
      };

      absl::optional<std::vector<int>> three_up =
          get_three(image_iter->MakeReverseRowIterator());
      absl::optional<std::vector<int>> three_down =
          get_three(image_iter->MakeForwardRowIterator());

      if (three_up.has_value() && three_down.has_value()) {
        std::vector<int> combined = {
            (*three_up)[2],                     //
            (*three_up)[1],                     //
            (*three_up)[0] + (*three_down)[0],  //
            (*three_down)[1],                   //
            (*three_down)[2],
        };

        std::cout << "three_up " << *three_up << "\n";
        std::cout << "three_down " << *three_down << "\n";
        std::cout << "combined " << combined << "\n";

        //   if (IsPositioningBlock(combined)) {
        out.emplace_back(std::make_pair(row, cand));
        // }
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

    auto row_candidates = processRow(&image_iter, row);
    candidates.insert(candidates.end(), row_candidates.begin(),
                      row_candidates.end());
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
