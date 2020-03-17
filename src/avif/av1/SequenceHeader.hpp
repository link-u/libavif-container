//
// Created by psi on 2020/01/05.
//

#pragma once

#include <vector>
#include <cstdint>
#include <optional>
#include "../img/Conversion.hpp"
#include "Header.hpp"

namespace avif::av1 {

struct SequenceHeader final {
  //
  uint8_t seqProfile{};
  bool stillPicture{};
  bool reducedStillPictureHeader{};

  bool timingInfoPresentFlag{};

  struct TimingInfo final {
    uint32_t numUnitsInDelayTick{};
    uint32_t timeScale{};
    bool equalPictureInterval{};
    uint32_t numTicksPerPicture{};
  };
  std::optional<TimingInfo> timingInfo{};
  bool decoderModelInfoPresentFlag{};

  struct DecoderModelInfo final {
    uint8_t bufferDelayLength{};
    uint32_t numUnitsInDecodingTick{};
    uint8_t bufferRemovalTimeLength{};
    uint8_t framePresentationTimeLength{};
  };
  std::optional<DecoderModelInfo> decoderModelInfo{};
  bool initialDisplayDelayPresentFlag{};
  uint8_t operatingPointsCnt{};

  struct OperatingPoint final {
    uint16_t idc{};
    uint8_t seqLevelIdx{};
    uint8_t seqTier{};
    bool decoderModelPresentFlag{};

    struct OperatingParametersInfo {
      uint64_t decoderBufferDelay{};
      uint64_t encoderBufferDelay{};
      bool lowDelayModeFlag{};
    };
    std::optional<OperatingParametersInfo> operatingParametersInfo;
    bool initialDisplayDelayPresent{};
    std::optional<uint8_t> initialDisplayDelay;
  };
  std::vector<OperatingPoint> operatingPoints{};
  uint8_t frameWidthBits{};
  uint8_t frameHeightBits{};
  uint32_t maxFrameWidth{};
  uint32_t maxFrameHeight{};
  bool frameIDNumbersPresentFlag{};
  std::optional<uint8_t> deltaFrameIDLength{};
  std::optional<uint8_t> additionalFrameIDLength{};
  bool use128x128Superblock{};
  bool enableFilterIntra{};
  bool enableIntraEdgeFilter{};
  bool enableInterintraCompound{};
  bool enableMaskedCompound{};
  bool enableWarpedMotion{};
  bool enableDualFilter{};
  bool enableOrderHint{};
  bool enableJNTComp{};
  bool enableRefFrameMVS{};
  bool seqChooseScreenContentTools{};

  constexpr static uint8_t SELECT_SCREEN_CONTENT_TOOLS = 2;
  uint8_t seqForceScreenContentTools{};

  bool seqChooseIntegerMV{};
  constexpr static uint8_t SELECT_INTEGER_MV = 2;
  bool seqForceIntegerMV{};
  uint8_t orderHintBits{};
  bool enableSuperres{};
  bool enableCDEF{};
  bool enableRestoration{};
  struct ColorConfig final {
    bool highBitdepth{};
    bool twelveBit{};
    bool monochrome{};
    bool colorDescriptionPresentFlag{};

    // 6.4.2. Color config semantics
    // color_primaries is an integer that is defined by the “Color primaries”
    // section of ISO/IEC 23091-4/ITU-T H.273.
    std::optional<avif::img::ColorPrimaries> colorPrimaries{};
    // transfer_characteristics is an integer that is defined by the “Transfer characteristics”
    // section of ISO/IEC 23091-4/ITU-T H.273.
    std::optional<avif::img::TransferCharacteristics> transferCharacteristics{};
    // matrix_coefficients is an integer that is defined by the “Matrix coefficients”
    // section of ISO/IEC 23091-4/ITU-T H.273.
    std::optional<avif::img::MatrixCoefficients> matrixCoefficients{};
    bool colorRange{};
    uint8_t subsamplingX{};
    uint8_t subsamplingY{};
    std::optional<uint8_t> chromaSamplePosition{};
    bool separateUVDeltaQ{};
  };
  ColorConfig colorConfig{};
  bool filmGrainParamsPresent{};
};

}
