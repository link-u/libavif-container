//
// Created by psi on 2019/11/25.
//

#pragma once

#include <vector>
#include "FullBox.hpp"

namespace avif {

struct ItemPropertyAssociation : public FullBox {
  uint32_t itemCount;
  struct Item {
    uint32_t itemID;
    struct Entry {
      bool essential;
      uint16_t propertyIndex;
    };
    uint8_t entryCount;
    std::vector<Entry> entries;
  };
  std::vector<Item> items;
};

}