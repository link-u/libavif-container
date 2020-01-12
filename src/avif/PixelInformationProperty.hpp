//
// Created by psi on 2019/11/25.
//

#pragma once

#include <vector>

#include "FullBox.hpp"

namespace avif {

struct PixelInformationProperty : FullBox {
  std::vector<uint8_t> bitsPerChannel;
};

}