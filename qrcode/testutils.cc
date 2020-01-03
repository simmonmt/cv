#include "qrcode/testutils.h"

#include <math.h>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <string>

#include "absl/memory/memory.h"

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

absl::variant<std::unique_ptr<QRCodeArray>, std::string>
ReadQRCodeArrayFromFile(const std::string& path) {
  std::ifstream input(path);
  if (!input.is_open()) {
    return "failed to open input";
  }

  std::vector<std::string> lines;
  std::string line;
  unsigned long width = 0;
  while (std::getline(input, line)) {
    width = std::max(width, line.size());
    lines.push_back(line);
  }

  auto array = absl::make_unique<QRCodeArray>(lines.size(), width);
  for (int y = 0; y < lines.size(); ++y) {
    const std::string& row = lines[y];
    for (int x = 0; x < width; ++x) {
      bool val;
      if (x >= row.size()) {
        val = false;
      } else {
        val = row[x] == 'X';
      }

      array->Set(Point(x, y), val);
    }
  }

  return std::move(array);
}

std::unique_ptr<QRCodeArray> ReadQRCodeArrayFromStrings(
    const std::vector<std::string> lines) {
  auto array = absl::make_unique<QRCodeArray>(lines.size(), lines.size());

  for (int y = 0; y < lines.size(); ++y) {
    const std::string& row = lines[y];
    for (int x = 0; x < row.size(); ++x) {
      bool val;
      if (x >= lines.size()) {
        val = false;
      } else {
        val = row[x] == 'X';
      }

      array->Set(Point(x, y), val);
    }
  }

  return array;
}

std::vector<std::string> QRCodeArrayToStrings(const QRCodeArray& array) {
  std::vector<std::string> out;

  for (int y = 0; y < array.height(); ++y) {
    std::string line;
    for (int x = 0; x < array.width(); ++x) {
      line += array.Get(Point(x, y)) ? "X" : " ";
    }
    out.push_back(line);
  }

  return out;
}
