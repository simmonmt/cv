#include "qrcode/runner.h"

namespace {

int run_len(absl::Span<const unsigned char> vals) {
  const unsigned char want = vals.front();
  int len = 1;
  for (int i = 1; i < vals.size() && vals[i] == want; i++) {
    ++len;
  }
  return len;
}

}  // namespace

Runner::Runner(absl::Span<const unsigned char> vals)
  : vals_(vals),
    start_(0) {}

absl::optional<std::vector<int>> Runner::Next(const int num, int* idx) {
  std::vector<int> lens(num);

  absl::Span<const unsigned char> left = vals_;
  for (int i = 0; i < lens.size(); ++i) {
    if (left.empty()) {
      return absl::nullopt;
    }
    int len = run_len(left);
    lens[i] = len;
    left = left.subspan(len);
  }

  *idx = start_;

  start_ += lens[0];
  vals_ = vals_.subspan(lens[0]);

  return lens;
}
