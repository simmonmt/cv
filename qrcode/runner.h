#ifndef _QRCODE_RUNNER_H_
#define _QRCODE_RUNNER_H_ 1

#include <map>
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
  int Get(int start);
  int Count(int start);
  void PurgeBefore(int start);

  absl::Span<const unsigned char> vals_;
  std::map<int, int> cache_;
  int start_;
};

#endif // _QRCODE_RUNNER_H_
