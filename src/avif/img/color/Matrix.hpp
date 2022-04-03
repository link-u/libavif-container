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

class IdentityConverter final {
public:
  constexpr IdentityConverter() = default;
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
class PrimariesConverter final {
public:
  PrimariesConverter() = delete;
  constexpr PrimariesConverter(float const Kr, float const Kb)
  :Kr(Kr)
  ,Kb(Kb)
  ,Kg(1.0f - Kr -Kb)
  ,Cb_B(2.0f * (1.0f - Kb))
  ,Cb_G(-2.0f * (1.0f - Kb) * Kb / Kg)
  ,Cr_R(2.0f * (1.0f - Kr))
  ,Cr_G(-2.0f * (1.0f - Kr) * Kr / Kg)
  {

  }
  const float Kr;
  const float Kb;
  const float Kg;

  constexpr void calcYUV(float const r, float const g, float const b, float* y, float* u, float* v) const {
    *y = Kr * r + Kg * g + Kb * b;
    if (u) {
      *u = 0.5f * (b - *y) / (1.0f - Kb);
    }
    if (v) {
      *v = 0.5f * (r - *y) / (1.0f - Kr);
    }
  }

  [[nodiscard]] constexpr std::tuple<float, float, float> calcRGB(float const y, float const u, float const v) const {
    float const r = y             + Cr_R * v;
    float const g = y + Cb_G * u  + Cr_G * v;
    float const b = y + Cb_B * u;
    return std::make_tuple(r, g, b);
  }

private:
  const float Cb_B;
  const float Cb_G;

  const float Cr_R;
  const float Cr_G;
};

class ReservedConverter final {
public:
  ReservedConverter() = delete;
  explicit constexpr ReservedConverter(MatrixCoefficients mat)
      :mat_(mat) {

  }
  void calcYUV(float /* r */, float /* g */, float /* b */, float* /* y */, float* /* u */, float* /* v */) const {
    throw std::logic_error(fmt::format("[TODO] MatrixCoefficients = {} is reserved for future ues by ISO/IEC.", static_cast<uint8_t>(mat_)));
  }
  [[nodiscard]] std::tuple<float, float, float> calcRGB(float /* y */, float /* u */, float /* v */) const {
    throw std::logic_error(fmt::format("[TODO] MatrixCoefficients = {} is reserved for future ues by ISO/IEC.", static_cast<uint8_t>(mat_)));
  }

private:
  const MatrixCoefficients mat_;
};

// Unspecified
class UnspecifiedConverter final {
public:
  UnspecifiedConverter() = delete;
  explicit constexpr UnspecifiedConverter(MatrixCoefficients mat)
      :mat_(mat) {
  }
  void calcYUV(float /* r */, float /* g */, float /* b */, float* /* y */, float* /* u */, float* /* v */) const {
    throw std::logic_error(fmt::format("[TODO] MatrixCoefficients = {} means 'unspecified'.", static_cast<uint8_t>(mat_)));
  }
  [[nodiscard]] std::tuple<float, float, float> calcRGB(float /* y */, float /* u */, float /* v */) const {
    throw std::logic_error(fmt::format("[TODO] MatrixCoefficients = {} means 'unspecified'.", static_cast<uint8_t>(mat_)));
  }

private:
  const MatrixCoefficients mat_;
};

// Fallback
class UnimplementedConverter final {
public:
  UnimplementedConverter() = delete;
  explicit constexpr UnimplementedConverter(MatrixCoefficients mat)
  :mat_(mat) {
  }
  void calcYUV(float /* r */, float /* g */, float /* b */, float* /* y */, float* /* u */, float* /* v */) const {
    throw std::logic_error(fmt::format("[TODO] MatrixCoefficients = {} is not implemented yet.", static_cast<uint8_t>(mat_)));
  }
  [[nodiscard]] std::tuple<float, float, float> calcRGB(float /* y */, float /* u */, float /* v */) const {
    throw std::logic_error(fmt::format("[TODO] MatrixCoefficients = {} is not implemented yet.", static_cast<uint8_t>(mat_)));
  }

private:
  const MatrixCoefficients mat_;
};

/*
 * ConverterFactory. It returns different struct according to MatrixCoefficients.
 */

// Generally, it is reserved.
template <MatrixCoefficients code>
struct ConverterFactory {
  static constexpr auto create() {
    return ReservedConverter(code);
  }
};

template <>
struct ConverterFactory<MatrixCoefficients::MC_IDENTITY> {
  static constexpr auto create() {
    return IdentityConverter();
  }
};

template <>
struct ConverterFactory<MatrixCoefficients::MC_BT_709> {
  static constexpr auto create() {
    return PrimariesConverter(0.2126f, 0.0722f);
  }
};

template <>
struct ConverterFactory<MatrixCoefficients::MC_RESERVED_3> {
  static constexpr auto create() {
    return ReservedConverter(MatrixCoefficients::MC_RESERVED_3);
  }
};

template <>
struct ConverterFactory<MatrixCoefficients::MC_FCC> {
  static constexpr auto create() {
    return PrimariesConverter(0.30f, 0.11f);
  }
};

template <>
struct ConverterFactory<MatrixCoefficients::MC_BT_470_B_G> {
  static constexpr auto create() {
    return PrimariesConverter(0.299f, 0.114f);
  }
};

template <>
struct ConverterFactory<MatrixCoefficients::MC_NSTC> {
  static constexpr auto create() {
    return PrimariesConverter(0.299f, 0.114f);
  }
};

template <>
struct ConverterFactory<MatrixCoefficients::MC_SMPTE_240> {
  static constexpr auto create() {
    return PrimariesConverter(0.212f, 0.087f);
  }
};

template <>
struct ConverterFactory<MatrixCoefficients::MC_SMPTE_YCGCO> {
  static constexpr auto create() {
    return UnimplementedConverter(MatrixCoefficients::MC_SMPTE_YCGCO);
  }
};

template <>
struct ConverterFactory<MatrixCoefficients::MC_BT_2020_NCL> {
  static constexpr auto create() {
    return PrimariesConverter(0.2627f, 0.0593f);
  }
};

template <>
struct ConverterFactory<MatrixCoefficients::MC_BT_2020_CL> {
  static constexpr auto create() {
    return UnimplementedConverter(MatrixCoefficients::MC_SMPTE_YCGCO);
  }
};

template <>
struct ConverterFactory<MatrixCoefficients::MC_SMPTE_2085> {
  static constexpr auto create() {
    return UnimplementedConverter(MatrixCoefficients::MC_SMPTE_2085);
  }
};

template <>
struct ConverterFactory<MatrixCoefficients::MC_CHROMAT_NCL> {
  static constexpr auto create() {
    return UnimplementedConverter(MatrixCoefficients::MC_CHROMAT_NCL);
  }
};

template <>
struct ConverterFactory<MatrixCoefficients::MC_CHROMAT_CL> {
  static constexpr auto create() {
    return UnimplementedConverter(MatrixCoefficients::MC_CHROMAT_CL);
  }
};

template <>
struct ConverterFactory<MatrixCoefficients::MC_BT_2100_ICTCP> {
  static constexpr auto create() {
    return UnimplementedConverter(MatrixCoefficients::MC_BT_2100_ICTCP);
  }
};

}
