//
// Created by psi on 2020/01/23.
//

#pragma once

#include "Box.hpp"

namespace avif {

struct CleanApertureBox final : public Box {
  uint32_t cleanApertureWidthN{};
  uint32_t cleanApertureWidthD{};

  uint32_t cleanApertureHeightN{};
  uint32_t cleanApertureHeightD{};

  uint32_t horizOffN{};
  uint32_t horizOffD{};

  uint32_t vertOffN{};
  uint32_t vertOffD{};
};

}