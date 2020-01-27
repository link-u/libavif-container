//
// Created by psi on 2019/11/24.
//

#pragma once

#include <optional>

#include "Box.hpp"
#include "FullBox.hpp"
#include "HandlerBox.hpp"
#include "ItemLocationBox.hpp"
#include "ItemPropertiesBox.hpp"
#include "ItemInfoBox.hpp"
#include "PrimaryItemBox.hpp"
#include "ItemReferenceBox.hpp"

namespace avif {

struct MetaBox final : public FullBox {
  HandlerBox handlerBox{};
  ItemPropertiesBox itemPropertiesBox{};
  ItemLocationBox itemLocationBox{};
  std::optional<PrimaryItemBox> primaryItemBox{};
  ItemInfoBox itemInfoBox{};
  std::optional<ItemReferenceBox> itemReferenceBox{};
};

}
