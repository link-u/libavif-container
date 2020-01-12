//
// Created by psi on 2019/11/25.
//

#pragma once

#include "FullBox.hpp"

namespace avif {

struct ItemLocationBox : public FullBox {
  uint8_t offsetSize;
  uint8_t lengthSize;
  uint8_t baseOffsetSize;
  uint8_t indexSize;
  struct Item {
    uint32_t itemID;
    uint8_t constructionMethod;
    uint16_t dataReferenceIndex;
    uint64_t baseOffset;
    struct Extent {
      uint64_t extentIndex;
      uint64_t extentOffset;
      uint64_t extentLength;
    };
    std::vector<Extent> extents;
  };
  std::vector<Item> items;
};

}