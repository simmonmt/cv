#ifndef _QRCODE_POINT_H_
#define _QRCODE_POINT_H_ 1

#include <iostream>

// A simple 2D point. This exists to limit the spread of OpenCV APIs (and
// linking of OpenCV libraries).
struct Point {
  Point(int x, int y) : x(x), y(y) {}

  bool operator==(const Point& p) const { return x == p.x && y == p.y; }

  int x;
  int y;
};

std::ostream& operator<<(std::ostream& stream, const Point& p);

#endif  // _QRCODE_POINT_H_
