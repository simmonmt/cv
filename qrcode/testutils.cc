#include "qrcode/testutils.h"

#include <math.h>
#include <cmath>

std::vector<unsigned char> MakeRun(std::vector<int> lens) {
  std::vector<unsigned char> out;
  for (int i = 0; i < lens.size(); ++i) {
    const int len = lens[i];
    std::vector<unsigned char> run(len, (i % 2 == 0) ? 255 : 0);
    out.insert(out.end(), run.begin(), run.end());
  }
  return out;
}

std::vector<PositioningPoints> MakeRotatedPositioningPoints(const Point& center,
                                                            double radius) {
  const int kNumSteps = 360;

  std::vector<PositioningPoints> out;
  for (int i = 0; i < kNumSteps; ++i) {
    double inc = (M_PI * 2) * (static_cast<double>(i) / kNumSteps);

    PositioningPoints points;
    std::vector<Point*> pp = {&points.bottom_left, &points.top_left,
                              &points.top_right};
    static const std::vector<double> kThetas = {inc + M_PI, inc + M_PI_2, inc};

    for (int i = 0; i < pp.size(); ++i) {
      const double theta = kThetas[i];
      Point* point = pp[i];

      point->x = std::sin(theta) * radius + center.x;
      point->y = std::cos(theta) * radius + center.y;
    }

    out.push_back(points);
  }

  return out;
}
