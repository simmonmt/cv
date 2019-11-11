#include "qrcode/pixel_iterator.h"

#include "opencv2/opencv.hpp"

PixelIterator<const unsigned char> PixelIteratorFromGrayImage(
    const cv::Mat& image) {
  return PixelIterator<const unsigned char>(image.ptr<unsigned char>(0),
                                            image.cols, image.rows);
}
