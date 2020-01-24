//
// Created by psi on 2020/01/24.
//

#pragma once

#include "Box.hpp"

namespace avif {

struct ImageMirrorBox final : public Box {
  enum class Axis : uint8_t {
    Vertical = 0,
    Horizontal = 1,
  };
  Axis axis{};
};

}
