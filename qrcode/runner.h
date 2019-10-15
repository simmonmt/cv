#ifndef _QRCODE_RUNNER_H_
#define _QRCODE_RUNNER_H_ 1

#include <vector>

#include "absl/types/optional.h"
#include "absl/types/span.h"

class Runner {
 public:
  Runner(absl::Span<const unsigned char> vals);
  ~Runner() = default;

  Runner(const Runner&) = delete;

  absl::optional<std::vector<int>> Next(const int num, int* idx);

 private:
  absl::Span<const unsigned char> vals_;
  int start_;
};

#endif // _QRCODE_RUNNER_H_
