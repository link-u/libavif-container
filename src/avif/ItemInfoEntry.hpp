//
// Created by psi on 2019/12/25.
//

#pragma once

#include "FullBox.hpp"
#include "ItemInfoExtension.hpp"

namespace avif {

struct ItemInfoEntry : public FullBox {
  uint32_t itemID;
  std::optional<std::string> itemType; //仕様書には記載がないが、これもバージョンによっては存在しない
  uint16_t itemProtectionIndex;
  std::string itemName;
  std::string contentType;
  std::optional<std::string> contentEncoding;
  std::optional<std::string> itemURIType; //仕様書には記載がないが、これもバージョンによっては存在しない
  std::variant<std::monostate, FDItemInfoExtension> itemInfoExtension;
};

}

