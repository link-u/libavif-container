//
// Created by psi on 2019/11/25.
//

#pragma once

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
  bool monochrome;
  bool chromaSubsamplingX;
  bool chromaSubsamplingY;
  bool chromaSamplePosition;

  bool initialPresentationDelayPresent;
  uint8_t initialPresentationDelayMinusOne;
};

struct AV1CodecConfigurationRecordBox : public Box {
  AV1CodecConfigurationRecord av1Config;
};

}