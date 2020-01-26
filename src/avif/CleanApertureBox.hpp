//
// Created by psi on 2020/01/23.
//

#pragma once

#include "Box.hpp"

namespace avif {

struct CleanApertureBox final : public Box {
  int32_t cleanApertureWidthN{};
  uint32_t cleanApertureWidthD{};

  int32_t cleanApertureHeightN{};
  uint32_t cleanApertureHeightD{};

  int32_t horizOffN{};
  uint32_t horizOffD{};

  int32_t vertOffN{};
  uint32_t vertOffD{};
};

}