#ifndef _QRCODE_RUNNER_H_
#define _QRCODE_RUNNER_H_ 1

#include <map>
#include <vector>

#include "absl/types/optional.h"
#include "absl/types/span.h"

// Runner finds ranges of consistent values.
//
// Given input 1,1,1,0,0,0,0,1,1, Runner is intended to return 3,4,2,
// as the sequence contains a run of 3 1's, then 4 0's, then 2 1's. It
// assumes the input values are 0 or non-zero, and won't distinguish
// between different non-zero values.
class Runner {
 public:
  // Does not assume ownership of the data pointed to by the span.
  Runner(absl::Span<const unsigned char> vals);
  ~Runner() = default;

  Runner(const Runner&) = delete;

  // Returns the next set of num runs, or absl::nullopt if there aren't that
  // many runs left. Assuming runs are returned, *idx will contain the index
  // relative to the start of vals (the ctor argument) that corresponds to the
  // beginning of the first run.
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
