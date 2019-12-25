//
// Created by psi on 2019/11/25.
//

#pragma once

#include <vector>
#include "Box.hpp"

namespace avif {

// https://aomediacodec.github.io/av1-isobmff/#av1codecconfigurationbox-section
struct AV1CodecConfigurationRecord {
  bool marker;
  uint8_t version;
  uint8_t seqProfile;
  uint8_t seqLevelIdx0;
  uint8_t seqTier0;
  bool highBitDepth;
  bool twelveBit;
  bool monochrome;
  uint8_t chromaSubsamplingX;
  uint8_t chromaSubsamplingY;
  uint8_t chromaSamplePosition;

  bool initialPresentationDelayPresent;
  uint8_t initialPresentationDelay;

  std::vector<uint8_t> configOBUs;
};

struct AV1CodecConfigurationRecordBox : public Box {
  AV1CodecConfigurationRecord av1Config;
};

}