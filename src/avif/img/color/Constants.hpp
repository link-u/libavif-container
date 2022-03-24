//
// Created by psi on 2022/01/10.
//

#pragma once
#include <cstdint>

namespace avif::img::color {

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
  CP_22 = 22, // No corresponding industry specification identified
};

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

enum class MatrixCoefficients : uint8_t {
  MC_IDENTITY = 0,
  MC_BT_709 = 1,
  MC_UNSPECIFIED = 2,
  MC_RESERVED_3 = 3,
  MC_FCC = 4,
  MC_BT_470_B_G = 5,
  MC_NSTC = 6,
  MC_SMPTE_240 = 7,
  MC_SMPTE_YCGCO = 8,
  MC_BT_2020_NCL = 9,
  MC_BT_2020_CL = 10,
  MC_SMPTE_2085 = 11,
  MC_CHROMAT_NCL = 12,
  MC_CHROMAT_CL = 13,
  MC_BT_2100_ICTCP = 14,
};

}
