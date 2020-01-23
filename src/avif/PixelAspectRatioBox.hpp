//
// Created by psi on 2019/11/25.
//

#pragma once

#include "Box.hpp"

namespace avif {

struct PixelAspectRatioBox final : public Box {
  uint32_t hSpacing{};
  uint32_t vSpacing{};
};

}