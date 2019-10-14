#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"

int main(int argc, char** argv) {
  constexpr char kWindowName[] = "Output";

  cv::namedWindow(kWindowName, 1);

  cv::Mat output = cv::Mat::zeros(120, 350, CV_8UC3);

  cv::imshow(kWindowName, output);
  cv::waitKey(0);

  return 0;
}
