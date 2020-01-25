//
// Created by psi on 2020/01/23.
//

#pragma once

#include "Box.hpp"

namespace avif {

struct ContentLightLevelBox final : public Box {
  uint16_t maxContentLightLevel{};
  uint16_t maxPicAverageLightLevel{};
};

}