//
// Created by psi on 2020/01/23.
//

#pragma once

#include "Box.hpp"

namespace avif {

struct ImageRotation final : public Box {
  uint8_t angle{};
};

}