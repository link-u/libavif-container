//
// Created by psi on 2020/01/25.
//

#pragma once

#include <array>
#include <cstdint>
#include "Box.hpp"

namespace avif {

struct MasteringDisplayColourVolumeBox final : public Box {
  std::array<uint16_t, 3> displayPrimariesX;
  std::array<uint16_t, 3> displayPrimariesY;
  uint16_t whitePointX;
  uint16_t whitePointY;
  uint32_t maxDisplayMasteringLuminance;
  uint32_t minDisplayMasteringLuminance;
};

}

