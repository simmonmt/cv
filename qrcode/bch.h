#ifndef _QRCODE_BCH_H_
#define _QRCODE_BCH_H_ 1

#include <memory>
#include <string>
#include <vector>

#include "absl/types/variant.h"

#include "qrcode/gf.h"

absl::variant<std::vector<bool>, std::string> DecodeBCH(
    const GF& gf, const std::vector<bool>& bits);

#endif  // _QRCODE_BCH_H_
