#pragma once

#include <variant>
#include <vector>
#include "Box.hpp"

namespace avif {

struct ColourInformationBox final : public Box {
  struct CICP final {
    // See ISO_IEC_23000-22_2019_Amd_2_2021(en)
    // p.5
    uint16_t colourPrimaries = 1;
    uint16_t transferCharacteristics = 13;
    uint16_t matrixCoefficients = 5 /* or 6 */;
    bool fullRangeFlag = true;
  };
  struct RestrictedICC {
    std::vector<uint8_t> payload;
  };
  struct UnrestrictedICC {
    std::vector<uint8_t> payload;
  };
  std::variant<std::monostate, CICP, RestrictedICC, UnrestrictedICC> profile;
};

}