//
// Created by psi on 2020/03/30.
//

#pragma once

#include <tuple>
#include <stdexcept>
#include <fmt/format.h>
#include "./Constants.hpp"

//
// See ISO/IEC 23091-4/ITU-T H.273,
// Section 8.3 Matrix coefficients
//

namespace avif::img::color {

struct IdentityConverter {
  static constexpr void calcYUV(float r, float g, float b, float* y, float* u, float* v) {
    *y = g;
    if(u) {
      *u = b;
    }
    if(v) {
      *v = r;
    }
  }
  [[nodiscard]] static constexpr std::tuple<float, float, float> calcRGB(float y, float u, float v) {
    return std::make_tuple(v, y, u);
  }
};

//
// This class corresponds to eq (38)-(40)
//
template <typename Self>
struct PrimariesConverter {
  static constexpr float Kr = Self::Kr;
  static constexpr float Kb = Self::Kb;
  static constexpr float Kg = (1.0f - Kr -Kb);
  static constexpr float Cb_B = (2.0f * (1.0f - Kb));
  static constexpr float Cb_G = (-2.0f * (1.0f - Kb) * Kb / Kg);
  static constexpr float Cr_R = (2.0f * (1.0f - Kr));
  static constexpr float Cr_G = (-2.0f * (1.0f - Kr) * Kr / Kg);

  static constexpr void calcYUV(float const r, float const g, float const b, float* y, float* u, float* v) {
    *y = Kr * r + Kg * g + Kb * b;
    if (u) {
      *u = 0.5f * (b - *y) / (1.0f - Kb);
    }
    if (v) {
      *v = 0.5f * (r - *y) / (1.0f - Kr);
    }
  }

  [[nodiscard]] static constexpr std::tuple<float, float, float> calcRGB(float const y, float const u, float const v) {
    float const r = y             + Cr_R * v;
    float const g = y + Cb_G * u  + Cr_G * v;
    float const b = y + Cb_B * u;
    return std::make_tuple(r, g, b);
  }
};


template <MatrixCoefficients mat>
struct ReservedConverter {
  static void calcYUV(float /* r */, float /* g */, float /* b */, float* /* y */, float* /* u */, float* /* v */) {
    throw std::logic_error(fmt::format("[TODO] MatrixCoefficients = {} is reserved for future ues by ISO/IEC.", static_cast<uint8_t>(mat)));
  }
  [[nodiscard]] static std::tuple<float, float, float> calcRGB(float /* y */, float /* u */, float /* v */) {
    throw std::logic_error(fmt::format("[TODO] MatrixCoefficients = {} is reserved for future ues by ISO/IEC.", static_cast<uint8_t>(mat)));
  }
};

// Unspecified
template <MatrixCoefficients mat>
struct UnspecifiedConverter {
  static void calcYUV(float /* r */, float /* g */, float /* b */, float* /* y */, float* /* u */, float* /* v */) {
    throw std::logic_error(fmt::format("[TODO] MatrixCoefficients = {} means 'unspecified'.", static_cast<uint8_t>(mat)));
  }
  [[nodiscard]] static std::tuple<float, float, float> calcRGB(float /* y */, float /* u */, float /* v */) {
    throw std::logic_error(fmt::format("[TODO] MatrixCoefficients = {} means 'unspecified'.", static_cast<uint8_t>(mat)));
  }
};

// Fallback
template <MatrixCoefficients mat>
struct UnimplementedConverter {
  static void calcYUV(float /* r */, float /* g */, float /* b */, float* /* y */, float* /* u */, float* /* v */) {
    throw std::logic_error(fmt::format("[TODO] MatrixCoefficients = {} is not implemented yet.", static_cast<uint8_t>(mat)));
  }
  [[nodiscard]] static std::tuple<float, float, float> calcRGB(float /* y */, float /* u */, float /* v */) {
    throw std::logic_error(fmt::format("[TODO] MatrixCoefficients = {} is not implemented yet.", static_cast<uint8_t>(mat)));
  }
};

/*
 * ConverterFactory. It returns different struct according to MatrixCoefficients.
 */

// Generally, it is unimplemented.
template <MatrixCoefficients mat>
struct ColorConverter final: public UnimplementedConverter<mat> {
};

template<>
struct ColorConverter<MatrixCoefficients::MC_IDENTITY> final: public IdentityConverter {
};

template <>
struct ColorConverter<MatrixCoefficients::MC_BT_709> final : public PrimariesConverter<ColorConverter<MatrixCoefficients::MC_BT_709>> {
  static constexpr float Kr = 0.2126f;
  static constexpr float Kb = 0.0722f;
};

template <>
struct ColorConverter<MatrixCoefficients::MC_RESERVED_3> final : public ReservedConverter<MatrixCoefficients::MC_RESERVED_3> {
};

template <>
struct ColorConverter<MatrixCoefficients::MC_FCC> : public PrimariesConverter<ColorConverter<MatrixCoefficients::MC_FCC>> {
  static constexpr float Kr = 0.30f;
  static constexpr float Kb = 0.11f;
};

template <>
struct ColorConverter<MatrixCoefficients::MC_BT_470_B_G> : public PrimariesConverter<ColorConverter<MatrixCoefficients::MC_BT_470_B_G>> {
  static constexpr float Kr = 0.299f;
  static constexpr float Kb = 0.114f;
};

template <>
struct ColorConverter<MatrixCoefficients::MC_NSTC> : public PrimariesConverter<ColorConverter<MatrixCoefficients::MC_NSTC>> {
  static constexpr float Kr = 0.299f;
  static constexpr float Kb = 0.114f;
};

template <>
struct ColorConverter<MatrixCoefficients::MC_SMPTE_240> : public PrimariesConverter<ColorConverter<MatrixCoefficients::MC_SMPTE_240>> {
  static constexpr float Kr = 0.212f;
  static constexpr float Kb = 0.087f;
};

template <>
struct ColorConverter<MatrixCoefficients::MC_SMPTE_YCGCO> : public UnimplementedConverter<MatrixCoefficients::MC_SMPTE_YCGCO> {
};

template <>
struct ColorConverter<MatrixCoefficients::MC_BT_2020_NCL> : public PrimariesConverter<ColorConverter<MatrixCoefficients::MC_BT_2020_NCL>> {
  static constexpr float Kr = 0.2627f;
  static constexpr float Kb = 0.0593f;
};

template <>
struct ColorConverter<MatrixCoefficients::MC_BT_2020_CL> : public UnimplementedConverter<MatrixCoefficients::MC_BT_2020_CL> {
};

template <>
struct ColorConverter<MatrixCoefficients::MC_SMPTE_2085> : public UnimplementedConverter<MatrixCoefficients::MC_SMPTE_2085> {
};

template <>
struct ColorConverter<MatrixCoefficients::MC_CHROMAT_NCL> : public UnimplementedConverter<MatrixCoefficients::MC_CHROMAT_NCL> {
};

template <>
struct ColorConverter<MatrixCoefficients::MC_CHROMAT_CL> : public UnimplementedConverter<MatrixCoefficients::MC_CHROMAT_CL> {
};

template <>
struct ColorConverter<MatrixCoefficients::MC_BT_2100_ICTCP> : public UnimplementedConverter<MatrixCoefficients::MC_BT_2100_ICTCP> {
};

}
