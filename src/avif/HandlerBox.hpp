//
// Created by psi on 2019/11/24.
//

#pragma once

#include <vector>
#include <string>

#include "FullBox.hpp"

namespace avif {

struct HandlerBox : public FullBox {
  std::string name;
  std::string handler;
};

}
