//
// Created by psi on 2019/11/25.
//

#pragma once

#include "Box.hpp"
#include "FullBox.hpp"

namespace avif {

struct ImageSpatialExtentsProperty : public FullBox {
  uint32_t imageWidth;
  uint32_t imageHeight;
};

}