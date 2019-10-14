#ifndef _QRCODE_RUNNER_H_
#define _QRCODE_RUNNER_H_ 1

#include "absl/types/optional.h"
#include "absl/types/span.h"

class Runner {
 public:
  Runner(absl::Span<unsigned char> vals)
    : vals_(vals) {}
  ~Runner() = default;

  Runner(const Runner&) = delete;

  absl::optional<std::vector<int>> Next();

 private:
  absl::Span<unsigned char> vals_;
};

#endif // _QRCODE_RUNNER_H_
