//
// Created by psi on 2020/01/27.
//

#pragma once

#include <variant>
#include <vector>
#include "FullBox.hpp"

namespace avif {

struct SingleItemTypeReferenceBox : public Box {
  uint16_t fromItemID{};
  std::vector<uint16_t> toItemIDs{};
};

struct SingleItemTypeReferenceBoxLarge : public Box {
  uint32_t fromItemID{};
  std::vector<uint32_t> toItemIDs{};
};

struct ItemReferenceBox : public FullBox {
  std::variant<std::vector<SingleItemTypeReferenceBox>, std::vector<SingleItemTypeReferenceBoxLarge>> references{};
};

}