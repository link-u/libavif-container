//
// Created by psi on 2020/01/23.
//

#pragma once

#include "Box.hpp"

namespace avif {

struct ImageRotationBox final : public Box {
  enum class Rotation : uint8_t {
    Rot0   = 0,
    Rot90  = 1,
    Rot180 = 2,
    Rot270 = 3,
  };
  Rotation angle{};
};

}