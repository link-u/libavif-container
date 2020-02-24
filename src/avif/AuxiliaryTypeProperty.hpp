//
// Created by psi on 2020/01/27.
//

#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include "FullBox.hpp"

namespace avif {

struct AuxiliaryTypeProperty : public FullBox {
  std::string auxType{};
  std::vector<uint8_t> auxSubtype{};
};

}