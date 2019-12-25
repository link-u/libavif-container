//
// Created by psi on 2019/12/25.
//

#pragma once

#include <vector>
#include "FullBox.hpp"
#include "ItemInfoEntry.hpp"

namespace avif {

struct ItemInfoBox : public FullBox {
  std::vector<ItemInfoEntry> itemInfos;
};

}