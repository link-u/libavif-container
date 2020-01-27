//
// Created by psi on 2020/01/27.
//

#pragma once

#include <cstdint>
#include "FullBox.hpp"

namespace avif {

struct RelativeLocationProperty final : public FullBox {
  uint32_t horizontalOffset{};
  uint32_t verticalOffset{};
};

}