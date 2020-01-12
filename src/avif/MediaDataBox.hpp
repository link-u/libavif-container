//
// Created by psi on 2019/11/24.
//

#pragma once

#include <vector>
#include "Box.hpp"

namespace avif {

struct MediaDataBox : public Box {
  size_t offset;
  size_t size;
};

}