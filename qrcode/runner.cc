#include "qrcode/runner.h"

#include <assert.h>
#include <iostream>

Runner::Runner(DirectionalIterator<const unsigned char> iter)
    : iter_(iter), iter_pos_(0), iter_empty_(false), start_(0) {}

int Runner::CountNext() {
  auto try_next = [&]() {
    bool ret = iter_.Next();
    if (!ret) {
      iter_empty_ = true;
    } else {
      ++iter_pos_;
    }
    return ret;
  };

  int len;
  const unsigned char want = iter_.Get();
  for (len = 1; try_next(); ++len) {
    if (iter_.Get() != want) {
      break;
    }
  }

  return len;
}

int Runner::Get(int start) {
  if (iter_empty_) {
    return 0;
  }

  auto iter = cache_.find(start);
  if (iter == cache_.end()) {
    // There's no entry for start in the cache, so we need to count the next run
    // of values.  Make sure the next run starts where we think it does. If
    // we're mistaken, we'd return an incorrect starting position.
    assert(start == iter_pos_);
    auto res = cache_.insert(std::make_pair(start, CountNext()));
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
