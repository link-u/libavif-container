//
// Created by psi on 2019/12/25.
//

#pragma once

#include <cstdint>
#include <vector>
#include <string>

namespace avif {

struct ItemInfoExtension {
protected:
  ItemInfoExtension() = default;
  ItemInfoExtension(ItemInfoExtension&&) = default;
  ItemInfoExtension(ItemInfoExtension const&) = default;
  ItemInfoExtension& operator=(ItemInfoExtension&&) = default;
  ItemInfoExtension& operator=(ItemInfoExtension const&) = default;

public:
  uint32_t extensionType;
};

struct FDItemInfoExtension final : public ItemInfoExtension {
  std::string contentLocation;
  std::string contentMD5;
  uint64_t contentLength;
  uint64_t transferLength;
  std::vector<uint32_t> groupIDs;
};

}
