//
// Created by psi on 2020/01/05.
//

#pragma once

#include <cstdint>
#include <optional>

namespace avif::av1 {

struct ExtensionHeader {
  uint8_t temporalID;
  uint8_t spatialID;
  uint8_t extensionHeaderReserved3bits;
};

struct Header {
  enum class Type : uint8_t {
    Reserved = 0,
    SequenceHeader = 1,
    TemporalDelimiter = 2,
    FrameHeader = 3,
    TileGroup = 4,
    Metadata = 5,
    Frame = 6,
    RedunduntFrameHeader = 7,
    TileList = 8,
    /* 9-14: reserved */
    Padding = 15,
  };
  bool forbidden;
  Type type;
  bool extensionFlag;
  bool hasSizeField;
  bool reserved1bit;
  std::optional<ExtensionHeader> extensionHeader;
};

}