#include "qrcode/qr_types.h"

std::ostream& operator<<(std::ostream& stream, const PositioningPoints& pp) {
  return stream << "PP<tl:" << pp.top_left << " tr:" << pp.top_right
                << " bl:" << pp.bottom_left << ">";
}
