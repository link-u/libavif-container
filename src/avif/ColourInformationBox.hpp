#pragma once

#include <variant>
#include <vector>
#include "Box.hpp"

namespace avif {

struct ColourInformationBox final : public Box {
  struct CICP final {
    uint16_t colourPrimaries{};
    uint16_t transferCharacteristics{};
    uint16_t matrixCoefficients{};
    bool fullRangeFlag{};
    // See ISO_IEC_23000-22_2019_Amd_2_2021(en)
    // p.5
    CICP()
    :colourPrimaries(1)
    ,transferCharacteristics(13)
    ,matrixCoefficients(5 /* or 6 */)
    ,fullRangeFlag(true)
    {
    }
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