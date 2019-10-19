#include "qrcode/runner.h"

Runner::Runner(absl::Span<const unsigned char> vals) : vals_(vals), start_(0) {}

int Runner::Count(int start) {
  if (start >= vals_.size()) {
    return 0;
  }

  const unsigned char want = vals_[start];
  int len = 1;
  for (int i = start + 1; i < vals_.size() && vals_[i] == want; i++) ++len;
  return len;
}

int Runner::Get(int start) {
  auto iter = cache_.find(start);
  if (iter == cache_.end()) {
    auto res = cache_.insert(std::make_pair(start, Count(start)));
    iter = res.first;
  }
  return iter->second;
}

void Runner::PurgeBefore(int start) {
  while (!cache_.empty() && cache_.begin()->first < start) {
    cache_.erase(cache_.begin());
  }
}

absl::optional<std::vector<int>> Runner::Next(const int num, int* idx) {
  if (num == 0) {
    return absl::nullopt;
  }

  std::vector<int> lens(num);
  lens[0] = Get(start_);
  *idx = start_;

  int next = start_ + lens[0];
  for (int i = 1; i < lens.size(); i++) {
    int len = Get(next);
    if (len == 0) {
      return absl::nullopt;
    }
    lens[i] = len;
    next += len;
  }

  start_ += lens[0];

  PurgeBefore(start_);

  return lens;
}
