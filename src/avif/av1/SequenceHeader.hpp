//
// Created by psi on 2020/01/05.
//

#pragma once

#include "Header.hpp"

namespace avif::av1 {

struct SequenceHeader final {
  //
  Header header{};
  uint8_t seqProfile{};
  bool stillPicture{};
  bool reducedStillPictureHeader{};

  bool timingInfoPresentFlag{};

  struct TimingInfo {
    uint32_t numUnitsInDelayTick;
    uint32_t timeScale;
    bool equalPictureInterval;
    uint32_t numTicksPerPicture;
  };
  std::optional<TimingInfo> timingInfo;
  bool decoderModelInfoPresentFlag{};

  struct DecoderModelInfo {
    uint8_t bufferDelayLength;
    uint32_t numUnitsInDecodingTick;
    uint8_t bufferRemovalTimeLength;
    uint8_t framePresentationTimeLength;
  };
  std::optional<DecoderModelInfo> decoderModelInfo;
  bool initialDisplayDelayPresentFlag{};
  uint8_t operatingPointsCnt{};

  struct OperatingPoint {
    uint16_t idc;
    uint8_t seqLevelIdx;
    uint8_t seqTier;
    bool decoderModelPresentFlag;

    struct OperatingParametersInfo {
      uint64_t decoderBufferDelay;
      uint64_t encoderBufferDelay;
      bool lowDelayModeFlag;
    };
    std::optional<OperatingParametersInfo> operatingParametersInfo;
    bool initialDisplayDelayPresent;
    std::optional<uint8_t> initialDisplayDelay;
  };
  std::vector<OperatingPoint> operatingPoints;
  uint8_t frameWidthBits;
  uint8_t frameHeightBits;
  uint32_t maxFrameWidth;
  uint32_t maxFrameHeight;
  bool frameIDNumbersPresentFlag;
  std::optional<uint8_t> deltaFrameIDLength;
  std::optional<uint8_t> additionalFrameIDLength;
  bool use128x128Superblock;
  bool enableFilterIntra;
  bool enableIntraEdgeFilter;
  bool enableInterintraCompound;
  bool enableMaskedCompound;
  bool enableWarpedMotion;
  bool enableDualFilter;
  bool enableOrderHint;
  bool enableJNTComp;
  bool enableRefFrameMVS;
  bool seqChooseScreenContentTools;

  constexpr static uint8_t SELECT_SCREEN_CONTENT_TOOLS = 2;
  uint8_t seqForceScreenContentTools;

  bool seqChooseIntegerMV;
  constexpr static uint8_t SELECT_INTEGER_MV = 2;
  bool seqForceIntegerMV;
  uint8_t orderHintBits;
  bool enableSuperres;
  bool enableCDEF;
  bool enableRestoration;
  struct ColorConfig {
    bool highBitdepth;
    bool twelveBit;
    bool monochrome;
    bool colorDescriptionPresentFlag;

    // 6.4.2. Color config semantics
    // color_primaries is an integer that is defined by the “Color primaries”
    // section of ISO/IEC 23091-4/ITU-T H.273.
    enum class ColorPrimaries : uint8_t {
      CP_BT_709 = 1,
      CP_UNSPECIFIED = 2,
      CP_BT_470_M = 4,
      CP_BT_470_B_G = 5,
      CP_BT_601 = 6,
      CP_SMPTE_240 = 7,
      CP_GENERIC_FILM = 8,
      CP_BT_2020 = 9,
      CP_XYZ = 10,
      CP_SMPTE_431 = 11,
      CP_SMPTE_432 = 12,
      CP_EBU_3213 = 22,
    };
    std::optional<ColorPrimaries> colorPrimaries;
    // transfer_characteristics is an integer that is defined by the “Transfer characteristics”
    // section of ISO/IEC 23091-4/ITU-T H.273.
    enum class TransferCharacteristics : uint8_t {
      TC_RESERVED_0 = 0,
      TC_BT_709 = 1,
      TC_UNSPECIFIED = 2,
      TC_RESERVED_3 = 3,
      TC_BT_470_M = 4,
      TC_BT_470_B_G = 5,
      TC_BT_601 = 6,
      TC_SMPTE_240 = 7,
      TC_LINEAR = 8,
      TC_LOG_100 = 9,
      TC_LOG_100_SQRT10 = 10,
      TC_IEC_61966 = 11,
      TC_BT_1361 = 12,
      TC_SRGB = 13,
      TC_BT_2020_10_BIT = 14,
      TC_BT_2020_12_BIT = 15,
      TC_SMPTE_2084 = 16,
      TC_SMPTE_428 = 17,
      TC_HLG = 18,
    };
    std::optional<TransferCharacteristics> transferCharacteristics;
    // matrix_coefficients is an integer that is defined by the “Matrix coefficients”
    // section of ISO/IEC 23091-4/ITU-T H.273.
    enum class MatrixCoefficients : uint8_t {
      MC_IDENTITY = 0,
      MC_BT_709 = 1,
      MC_UNSPECIFIED = 2,
      MC_RESERVED_3 = 3,
      MC_FCC = 4,
      MC_BT_470_B_G = 5,
      MC_BT_601 = 6,
      MC_SMPTE_240 = 7,
      MC_SMPTE_YCGCO = 8,
      MC_BT_2020_NCL = 9,
      MC_BT_2020_CL = 10,
      MC_SMPTE_2085 = 11,
      MC_CHROMAT_NCL = 12,
      MC_CHROMAT_CL = 13,
      MC_ICTCP = 14,
    };
    std::optional<MatrixCoefficients> matrixCoefficients;
    bool colorRange;
    uint8_t subsamplingX;
    uint8_t subsamplingY;
    std::optional<uint8_t> chromaSamplePosition;
    bool separateUVDeltaQ;
  };
  ColorConfig colorConfig;
  bool filmGrainParamsPresent;
};

}
