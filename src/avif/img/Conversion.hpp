//
// Created by psi on 2020/01/21.
//

#pragma once

#include <cstdint>
#include <cmath>
#include <tuple>
#include "../../../external/tinyformat/tinyformat.h"
#include "Spec.hpp"
#include "Image.hpp"

namespace avif::img {

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

template <typename T, bool subX, bool subY>
struct ChromaSampler {
  static_assert(!std::is_pointer<T>::value);
  static constexpr bool isConst = std::is_const<T>::value;
  using LineType = typename std::conditional<isConst, const uint8_t, uint8_t>::type;
  constexpr LineType* nextLine(LineType* const currentLine, size_t const stride, size_t const y) {
    return subY                   ? (y % 2 == 1 ? (currentLine + stride) : currentLine) :
                                    (currentLine + stride);
  }
  constexpr T* pixelInLine(T* currentLine, size_t const x) {
    return subX                   ? &currentLine[x/2] :
                                    &currentLine[x];
  }
};

namespace detail {

template <MatrixCoefficients mat>
struct Converter {
  void calcYUV(float r, float g, float b, float* y, float* u, float* v) {
    throw std::logic_error(tfm::format("[TODO] MatrixCoefficients = %d is not implemented yet.", static_cast<uint8_t>(mat)));
  }
  std::tuple<float, float, float> calcRGB(float y, float u, float v) {
    throw std::logic_error(tfm::format("[TODO] MatrixCoefficients = %d is not implemented yet.", static_cast<uint8_t>(mat)));
  }
};

template <>
struct Converter<MatrixCoefficients::MC_IDENTITY> {
  void calcYUV(float r, float g, float b, float* y, float* u, float* v) {
    *y = g;
    if(u) {
      *u = b;
    }
    if(v) {
      *v = r;
    }
  }
  std::tuple<float, float, float> calcRGB(float y, float u, float v) {
    return {v, y, u};
  }
};

template <float const& Kr, float const& Kb>
struct KrKbConverter {
  static constexpr float Kg = 1.0f - Kr -Kb;
  static constexpr float Cb_B =  2.0f * (1.0f - Kb);
  static constexpr float Cb_G = -2.0f * (1.0f - Kb) * Kb / Kg;

  static constexpr float Cr_R =  2.0f * (1.0f - Kr);
  static constexpr float Cr_G = -2.0f * (1.0f - Kr) * Kr / Kg;

  constexpr void calcYUV(float r, float g, float b, float* y, float* u, float* v) const {
    *y = Kr * r + Kg * g + Kb * b;
    if (u) {
      *u = 0.5f * (b - *y) / (1.0f - Kb);
    }
    if (v) {
      *v = 0.5f * (r - *y) / (1.0f - Kr);
    }
  }
  constexpr std::tuple<float, float, float> calcRGB(float y, float u, float v) const {
    float const r = y             + Cr_R * v;
    float const g = y + Cb_G * u  + Cr_G * v;
    float const b = y + Cb_B * u;
    return std::make_tuple(r, g, b);
  }
};

template <>
struct Converter<MatrixCoefficients::MC_BT_709> {
  constexpr static float Kr = 0.2126f;
  constexpr static float Kb = 0.0722f;
  constexpr static KrKbConverter<Kr, Kb> impl = {};
  void calcYUV(float r, float g, float b, float* y, float* u, float* v) {
    impl.calcYUV(r, g, b, y, u, v);
  }
  std::tuple<float, float, float> calcRGB(float y, float u, float v) {
    return impl.calcRGB(y, u, v);
  }
};

template <>
struct Converter<MatrixCoefficients::MC_FCC> {
  constexpr static float Kr = 0.30f;
  constexpr static float Kb = 0.11f;
  constexpr static KrKbConverter<Kr, Kb> impl = {};
  void calcYUV(float r, float g, float b, float* y, float* u, float* v) {
    impl.calcYUV(r, g, b, y, u, v);
  }
  std::tuple<float, float, float> calcRGB(float y, float u, float v) {
    return impl.calcRGB(y, u, v);
  }
};

template <>
struct Converter<MatrixCoefficients::MC_BT_470_B_G> {
  constexpr static float Kr = 0.299f;
  constexpr static float Kb = 0.114f;
  constexpr static KrKbConverter<Kr, Kb> impl = {};
  void calcYUV(float r, float g, float b, float* y, float* u, float* v) {
    impl.calcYUV(r, g, b, y, u, v);
  }
  std::tuple<float, float, float> calcRGB(float y, float u, float v) {
    return impl.calcRGB(y, u, v);
  }
};

template <>
struct Converter<MatrixCoefficients::MC_BT_601> {
  constexpr static float Kr = 0.299f;
  constexpr static float Kb = 0.114f;
  constexpr static KrKbConverter<Kr, Kb> impl = {};
  void calcYUV(float r, float g, float b, float* y, float* u, float* v) {
    impl.calcYUV(r, g, b, y, u, v);
  }
  std::tuple<float, float, float> calcRGB(float y, float u, float v) {
    return impl.calcRGB(y, u, v);
  }
};

template <>
struct Converter<MatrixCoefficients::MC_SMPTE_240> {
  constexpr static float Kr = 0.212f;
  constexpr static float Kb = 0.087f;
  constexpr static KrKbConverter<Kr, Kb> impl = {};
  void calcYUV(float r, float g, float b, float* y, float* u, float* v) {
    impl.calcYUV(r, g, b, y, u, v);
  }
  std::tuple<float, float, float> calcRGB(float y, float u, float v) {
    return impl.calcRGB(y, u, v);
  }
};

template <>
struct Converter<MatrixCoefficients::MC_BT_2020_NCL> {
  constexpr static float Kr = 0.2627f;
  constexpr static float Kb = 0.0593f;
  constexpr static KrKbConverter<Kr, Kb> impl = {};
  void calcYUV(float r, float g, float b, float* y, float* u, float* v) {
    impl.calcYUV(r, g, b, y, u, v);
  }
  std::tuple<float, float, float> calcRGB(float y, float u, float v) {
    return impl.calcRGB(y, u, v);
  }
};


template <MatrixCoefficients matrixCoefficients, size_t rgbBits, size_t yuvBits, bool isMonoYUV, bool isFullRange>
constexpr void calcYUV(uint16_t const ir, uint16_t const ig, uint16_t const ib, typename avif::img::spec::YUV<yuvBits>::Type* dstY, typename avif::img::spec::YUV<yuvBits>::Type* dstU, typename avif::img::spec::YUV<yuvBits>::Type* dstV) {
  using Quantizer = typename avif::img::spec::Quantizer<rgbBits, yuvBits, isFullRange>;
  using RGBSpec = typename avif::img::spec::RGB<rgbBits>;
  float const r = static_cast<float>(ir) / RGBSpec::max;
  float const g = static_cast<float>(ig) / RGBSpec::max;
  float const b = static_cast<float>(ib) / RGBSpec::max;

  Converter<matrixCoefficients> converter;
  if (isMonoYUV) {
    float y = 0;
    converter.calcYUV(r,g,b, &y, nullptr, nullptr);
    *dstY = Quantizer::quantizeLuma(y);
  } else {
    float y = {};
    float u = {};
    float v = {};
    converter.calcYUV(r,g,b, &y, &u, &v);
    *dstY = Quantizer::quantizeLuma(y);
    *dstU = Quantizer::quantizeChroma(u);
    *dstV = Quantizer::quantizeChroma(v);
  }
}

template <MatrixCoefficients matrixCoefficients, size_t rgbBits, size_t yuvBits, bool isMonoYUV, bool isFullRange>
constexpr std::tuple<typename avif::img::spec::RGB<rgbBits>::Type, typename avif::img::spec::RGB<rgbBits>::Type, typename avif::img::spec::RGB<rgbBits>::Type> calcRGB(typename avif::img::spec::YUV<yuvBits>::Type const* srcY, typename avif::img::spec::YUV<yuvBits>::Type const* srcU, typename avif::img::spec::YUV<yuvBits>::Type const* srcV) {
  using avif::img::spec::clamp;
  using Quantizer = typename avif::img::spec::Quantizer<rgbBits, yuvBits, isFullRange>;
  using RGBSpec = typename avif::img::spec::RGB<rgbBits>;
  using YUVSpec = typename avif::img::spec::YUV<yuvBits>;

  using RGBType = typename RGBSpec::Type;

  auto const y = Quantizer::dequantizeLuma(*srcY);
  auto const u = isMonoYUV ? 0.0f : Quantizer::dequantizeChroma(*srcU);
  auto const v = isMonoYUV ? 0.0f : Quantizer::dequantizeChroma(*srcV);

  Converter<matrixCoefficients> converter;
  auto const [r, g, b] = converter.calcRGB(y, u, v);

  auto const ir = static_cast<RGBType>(clamp<int>(static_cast<int>(std::round(r * RGBSpec::max)), 0, RGBSpec::max));
  auto const ig = static_cast<RGBType>(clamp<int>(static_cast<int>(std::round(g * RGBSpec::max)), 0, RGBSpec::max));
  auto const ib = static_cast<RGBType>(clamp<int>(static_cast<int>(std::round(b * RGBSpec::max)), 0, RGBSpec::max));
  return std::make_tuple(ir, ig, ib);
}

// MonochromeYUV version
template <MatrixCoefficients matrixCoefficients, uint8_t rgbBits, uint8_t yuvBits, bool fromMonoRGB, bool isFullRange>
void constexpr convertFromRGB(size_t width, size_t height, uint8_t bytesPerPixel, uint8_t const* src, size_t const stride, uint8_t* const dstY, size_t const strideY) {
  using avif::img::spec::clamp;
  using RGBSpec = typename avif::img::spec::RGB<rgbBits>;
  using YUVSpec = typename avif::img::spec::YUV<yuvBits>;

  using RGBType = typename RGBSpec::Type const;
  using YUVType = typename YUVSpec::Type;

  uint8_t* lineY = dstY;
  uint8_t const* line = src;
  for(size_t y = 0; y < height; ++y) {
    uint8_t const *ptr = line;
    auto* ptrY = reinterpret_cast<YUVType*>(lineY);
    for (size_t x = 0; x < width; ++x) {
      if(fromMonoRGB) {
        uint16_t const mono = reinterpret_cast<RGBType const *>(ptr)[0];
        calcYUV<matrixCoefficients, rgbBits, yuvBits, true, isFullRange>(mono, mono, mono, &ptrY[x], nullptr, nullptr);
      } else {
        uint16_t const r = reinterpret_cast<RGBType const *>(ptr)[0];
        uint16_t const g = reinterpret_cast<RGBType const *>(ptr)[1];
        uint16_t const b = reinterpret_cast<RGBType const *>(ptr)[2];
        calcYUV<matrixCoefficients, rgbBits, yuvBits, true, isFullRange>(r, g, b, &ptrY[x], nullptr, nullptr);
      }
      ptr += bytesPerPixel;
    }
    lineY += strideY;
    line += stride;
  }
}

template <MatrixCoefficients matrixCoefficients, uint8_t rgbBits, uint8_t yuvBits, bool fromMonoRGB, bool isFullRange, bool subX, bool subY>
void constexpr convertFromRGB(size_t width, size_t height, uint8_t bytesPerPixel, uint8_t const* src, size_t const stride, uint8_t* const dstY, size_t const strideY, uint8_t* const dstU, size_t const strideU, uint8_t* const dstV, size_t const strideV) {
  using avif::img::spec::clamp;
  using RGBSpec = typename avif::img::spec::RGB<rgbBits>;
  using YUVSpec = typename avif::img::spec::YUV<yuvBits>;

  using RGBType = typename RGBSpec::Type const;
  using YUVType = typename YUVSpec::Type;

  using ChromaSampler = typename avif::img::ChromaSampler<YUVType, subX, subY>;
  ChromaSampler sampler;

  uint8_t* lineY = dstY;
  uint8_t* lineU = dstU;
  uint8_t* lineV = dstV;
  uint8_t const* line = src;
  for(size_t y = 0; y < height; ++y) {
    uint8_t const *ptr = line;
    auto* ptrY = reinterpret_cast<YUVType*>(lineY);
    auto* ptrU = reinterpret_cast<YUVType*>(lineU);
    auto* ptrV = reinterpret_cast<YUVType*>(lineV);
    for (size_t x = 0; x < width; ++x) {
      if(fromMonoRGB) {
        uint16_t const mono = reinterpret_cast<RGBType const *>(ptr)[0];
        calcYUV<matrixCoefficients, rgbBits, yuvBits, false, isFullRange>(mono, mono, mono, &ptrY[x], sampler.pixelInLine(ptrU, x), sampler.pixelInLine(ptrV, x));
      } else {
        uint16_t const r = reinterpret_cast<RGBType const *>(ptr)[0];
        uint16_t const g = reinterpret_cast<RGBType const *>(ptr)[1];
        uint16_t const b = reinterpret_cast<RGBType const *>(ptr)[2];
        calcYUV<matrixCoefficients, rgbBits, yuvBits, false, isFullRange>(r, g, b, &ptrY[x], sampler.pixelInLine(ptrU, x), sampler.pixelInLine(ptrV, x));
      }
      ptr += bytesPerPixel;
    }
    lineY += strideY;
    lineU = sampler.nextLine(lineU, strideU, y);
    lineV = sampler.nextLine(lineV, strideV, y);
    line += stride;
  }
}

// MonochromeYUV version
template <MatrixCoefficients matrixCoefficients, uint8_t rgbBits, uint8_t yuvBits, bool toMonoRGB, bool isFullRange>
void constexpr convertFromYUV(size_t width, size_t height, uint8_t bytesPerPixel, uint8_t* dst, size_t stride, uint8_t const* srcY, size_t strideY) {
  using RGBSpec = typename avif::img::spec::RGB<rgbBits>;
  using YUVSpec = typename avif::img::spec::YUV<yuvBits>;

  using YUVType = typename YUVSpec::Type const;
  using RGBType = typename RGBSpec::Type;

  uint8_t const* lineY = srcY;
  uint8_t* line = dst;
  for(size_t y = 0; y < height; ++y) {
    uint8_t* ptr = line;
    auto const* ptrY = reinterpret_cast<YUVType*>(lineY);
    for (size_t x = 0; x < width; ++x) {
      if(toMonoRGB) {
        RGBType& mono = reinterpret_cast<RGBType*>(ptr)[0];
        std::tie(mono, mono, mono) = calcRGB<matrixCoefficients, rgbBits, yuvBits, true, isFullRange>(&ptrY[x], nullptr, nullptr);
      } else {
        RGBType& r = reinterpret_cast<RGBType*>(ptr)[0];
        RGBType& g = reinterpret_cast<RGBType*>(ptr)[1];
        RGBType& b = reinterpret_cast<RGBType*>(ptr)[2];
        std::tie(r,g,b) = calcRGB<matrixCoefficients, rgbBits, yuvBits, true, isFullRange>(&ptrY[x], nullptr, nullptr);
      }
      ptr += bytesPerPixel;
    }
    lineY += strideY;
    line += stride;
  }
}

template <MatrixCoefficients matrixCoefficients, uint8_t rgbBits, uint8_t yuvBits, bool toMonoRGB, bool isFullRange, bool subX, bool subY>
void constexpr convertFromYUV(size_t width, size_t height, uint8_t bytesPerPixel, uint8_t* dst, size_t stride, uint8_t const* srcY, size_t strideY, uint8_t const* srcU, size_t strideU, uint8_t const* srcV, size_t strideV) {
  using RGBSpec = typename avif::img::spec::RGB<rgbBits>;
  using YUVSpec = typename avif::img::spec::YUV<yuvBits>;

  using YUVType = typename YUVSpec::Type const;
  using RGBType = typename RGBSpec::Type;
  using ChromaSampler = typename avif::img::ChromaSampler<YUVType, subX, subY>;
  ChromaSampler sampler;

  uint8_t const* lineY = srcY;
  uint8_t const* lineU = srcU;
  uint8_t const* lineV = srcV;
  uint8_t* line = dst;
  for(size_t y = 0; y < height; ++y) {
    uint8_t* ptr = line;
    auto const* ptrY = reinterpret_cast<YUVType*>(lineY);
    auto const* ptrU = reinterpret_cast<YUVType*>(lineU);
    auto const* ptrV = reinterpret_cast<YUVType*>(lineV);
    for (size_t x = 0; x < width; ++x) {
      if(toMonoRGB) {
        RGBType& mono = reinterpret_cast<RGBType*>(ptr)[0];
        std::tie(mono, mono, mono) = calcRGB<matrixCoefficients, rgbBits, yuvBits, false, isFullRange>(&ptrY[x], sampler.pixelInLine(ptrU, x), sampler.pixelInLine(ptrV, x));
      } else {
        RGBType& r = reinterpret_cast<RGBType*>(ptr)[0];
        RGBType& g = reinterpret_cast<RGBType*>(ptr)[1];
        RGBType& b = reinterpret_cast<RGBType*>(ptr)[2];
        std::tie(r,g,b) = calcRGB<matrixCoefficients, rgbBits, yuvBits, false, isFullRange>(&ptrY[x], sampler.pixelInLine(ptrU, x), sampler.pixelInLine(ptrV, x));
      }
      ptr += bytesPerPixel;
    }
    lineY += strideY;
    lineU = sampler.nextLine(lineU, strideU, y);
    lineV = sampler.nextLine(lineV, strideV, y);
    line += stride;
  }
}

}

template <MatrixCoefficients matrixCoefficients, uint8_t rgbBits, uint8_t yuvBits, bool toMonoRGB, bool isFullRange>
struct FromRGB final {
  void toI400(Image<rgbBits>& src, uint8_t* dstY, size_t strideY) {
    detail::convertFromRGB<matrixCoefficients, rgbBits, yuvBits, toMonoRGB, isFullRange>(src.width(), src.height(), src.bytesPerPixel(), src.data(), src.stride(), dstY, strideY);
  }
  void toI444(Image<rgbBits> const& src, uint8_t* dstY, size_t strideY, uint8_t* dstU, size_t strideU, uint8_t* dstV, size_t strideV) {
    detail::convertFromRGB<matrixCoefficients, rgbBits, yuvBits, toMonoRGB, isFullRange, false, false>(src.width(), src.height(), src.bytesPerPixel(), src.data(), src.stride(), dstY, strideY, dstU, strideU, dstV, strideV);
  }
  void toI422(Image<rgbBits> const& src, uint8_t* dstY, size_t strideY, uint8_t* dstU, size_t strideU, uint8_t* dstV, size_t strideV){
    detail::convertFromRGB<matrixCoefficients, rgbBits, yuvBits, toMonoRGB, isFullRange, true, false>(src.width(), src.height(), src.bytesPerPixel(), src.data(), src.stride(), dstY, strideY, dstU, strideU, dstV, strideV);
  }
  void toI420(Image<rgbBits> const& src, uint8_t* dstY, size_t strideY, uint8_t* dstU, size_t strideU, uint8_t* dstV, size_t strideV){
    detail::convertFromRGB<matrixCoefficients, rgbBits, yuvBits, toMonoRGB, isFullRange, true, true>(src.width(), src.height(), src.bytesPerPixel(), src.data(), src.stride(), dstY, strideY, dstU, strideU, dstV, strideV);
  }
};

template <MatrixCoefficients matrixCoefficients, uint8_t rgbBits, uint8_t yuvBits, bool isFullRange>
struct FromAlpha final {
  void toI400(Image<rgbBits>& src, uint8_t* dstY, size_t strideY) {
    switch(src.pixelOrder()) {
      case avif::img::PixelOrder::MonoA:
      case avif::img::PixelOrder::RGBA:
        detail::convertFromRGB<matrixCoefficients, rgbBits, yuvBits, true, isFullRange>(src.width(), src.height(), src.bytesPerPixel(), src.data() + (src.numComponents() - 1) * src.bytesPerComponent(), src.stride(), dstY, strideY);
        break;
      case avif::img::PixelOrder::Mono:
        throw std::domain_error("Cannot separate Alpha from Mono image.");
      case avif::img::PixelOrder::RGB:
        throw std::domain_error("Cannot separate Alpha from RGB image.");
    }
  }
};

template <MatrixCoefficients matrixCoefficients, uint8_t rgbBits, uint8_t yuvBits, bool toMonoRGB, bool isFullRange>
struct ToRGB final {
  void fromI400(Image<rgbBits>& dst, uint8_t* srcY, size_t strideY) {
    detail::convertFromYUV<matrixCoefficients, rgbBits, yuvBits, toMonoRGB, isFullRange>(dst.width(), dst.height(), dst.bytesPerPixel(), dst.data(), dst.stride(), srcY, strideY);
  }
  void fromI444(Image<rgbBits>& dst, uint8_t* srcY, size_t strideY, uint8_t* srcU, size_t strideU, uint8_t* srcV, size_t strideV) {
    detail::convertFromYUV<matrixCoefficients, rgbBits, yuvBits, toMonoRGB, isFullRange, false, false>(dst.width(), dst.height(), dst.bytesPerPixel(), dst.data(), dst.stride(), srcY, strideY, srcU, strideU, srcV, strideV);
  }
  void fromI422(Image<rgbBits>& dst, uint8_t* srcY, size_t strideY, uint8_t* srcU, size_t strideU, uint8_t* srcV, size_t strideV){
    detail::convertFromYUV<matrixCoefficients, rgbBits, yuvBits, toMonoRGB, isFullRange, true, false>(dst.width(), dst.height(), dst.bytesPerPixel(), dst.data(), dst.stride(), srcY, strideY, srcU, strideU, srcV, strideV);
  }
  void fromI420(Image<rgbBits>& dst, uint8_t* srcY, size_t strideY, uint8_t* srcU, size_t strideU, uint8_t* srcV, size_t strideV){
    detail::convertFromYUV<matrixCoefficients, rgbBits, yuvBits, toMonoRGB, isFullRange, true, true>(dst.width(), dst.height(), dst.bytesPerPixel(), dst.data(), dst.stride(), srcY, strideY, srcU, strideU, srcV, strideV);
  }
};
template <MatrixCoefficients matrixCoefficients, uint8_t rgbBits, uint8_t yuvBits, bool isFullRange>
struct ToAlpha final {
  void fromI400(Image<rgbBits>& dst, uint8_t* srcY, size_t strideY) {
    switch(dst.pixelOrder()) {
      case avif::img::PixelOrder::MonoA:
      case avif::img::PixelOrder::RGBA:
        detail::convertFromYUV<matrixCoefficients, rgbBits, yuvBits, true, isFullRange>(dst.width(), dst.height(), dst.bytesPerPixel(), dst.data() + (dst.numComponents() - 1) * dst.bytesPerComponent(), dst.stride(), srcY, strideY);
        break;
      case avif::img::PixelOrder::Mono:
        throw std::domain_error("Cannot store Alpha to Mono image.");
      case avif::img::PixelOrder::RGB:
        throw std::domain_error("Cannot store Alpha to RGB image.");
    }
  }
};

}
