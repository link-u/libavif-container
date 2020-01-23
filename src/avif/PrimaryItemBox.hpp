//
// Created by psi on 2020/01/23.
//

#pragma once


#include "Box.hpp"

namespace avif {

struct PrimaryItemBox final : FullBox {
  uint32_t itemID{};
};

}
