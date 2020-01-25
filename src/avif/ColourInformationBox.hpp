#pragma once

#include <variant>
#include <vector>
#include "Box.hpp"

namespace avif {

struct ColourInformationBox final : public Box {
  struct NCLX {
    uint16_t colourPrimaries{};
    uint16_t transferCharacteristics{};
    uint16_t matrixCoefficients{};
    bool fullRangeFlag{};
  };
  struct RestrictedICC {
    std::vector<uint8_t> payload;
  };
  struct UnrestrictedICC {
    std::vector<uint8_t> payload;
  };
  std::variant<std::monostate, NCLX, RestrictedICC, UnrestrictedICC> profile;
};

}