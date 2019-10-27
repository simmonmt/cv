#include <iostream>
#include <vector>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"

ABSL_FLAG(int, steps, 360, "num steps");
ABSL_FLAG(double, radius, 100, "radius");

int main(int argc, char** argv) {
  absl::ParseCommandLine(argc, argv);

  const int num_steps = absl::GetFlag(FLAGS_steps);
  const double r = absl::GetFlag(FLAGS_radius);
  for (int i = 0; i < num_steps; ++i) {
    double inc = (M_PI * 2) * (static_cast<double>(i) / num_steps);

    std::vector<double> thetas = {inc, inc + M_PI_2, inc + M_PI};

    for (int i = 0; i < thetas.size(); ++i) {
      const double theta = thetas[i];
      double x = std::sin(theta) * r;
      double y = std::cos(theta) * r;

      if (i != 0) {
        std::cout << ",";
      }
      std::cout << x << "," << y;
    }
    std::cout << "\n";
  }

  return 0;
}
