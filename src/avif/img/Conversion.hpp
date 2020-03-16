//
// Created by psi on 2020/01/21.
//

#pragma once

#include <cstdint>
#include <cmath>
#include <tuple>
#include "../../../external/tinyformat/tinyformat.h"
#include "../av1/SequenceHeader.hpp"
#include "Spec.hpp"
#include "Image.hpp"

namespace avif::img {

using MatrixType = avif::av1::SequenceHeader::ColorConfig::MatrixCoefficients;

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

template <MatrixType mat>
struct Converter {
  void calcYUV(float r, float g, float b, float* y, float* u, float* v) {
    throw std::logic_error(tfm::format("[TODO] MatrixCoefficients = %d is not implemented yet.", static_cast<uint8_t>(mat)));
  }
  std::tuple<float, float, float> calcRGB(float y, float u, float v) {
    throw std::logic_error(tfm::format("[TODO] MatrixCoefficients = %d is not implemented yet.", static_cast<uint8_t>(mat)));
  }
};

template <>
struct Converter<MatrixType::MC_BT_2020_NCL> {
  void calcYUV(float r, float g, float b, float* y, float* u, float* v) {
    *y = 0.2627f * r + 0.6780f * g + 0.0593f * b;
    if (u) {
      *u = (b - *y) / 1.8814f;
    }
    if (v) {
      *v = (r - *y) / 1.4746f;
    }
  }
  std::tuple<float, float, float> calcRGB(float y, float u, float v) {
    float const r = y                    + (+1.47460f) * v;
    float const g = y + (-0.16455f) * u  + (-0.57135f) * v;
    float const b = y + (+1.88140f) * u;
    return std::make_tuple(r, g, b);
  }
};

template <MatrixType matrixType, size_t rgbBits, size_t yuvBits, bool isMonoYUV, bool isFullRange>
constexpr void calcYUV(uint16_t const ir, uint16_t const ig, uint16_t const ib, typename avif::img::spec::YUV<yuvBits>::Type* dstY, typename avif::img::spec::YUV<yuvBits>::Type* dstU, typename avif::img::spec::YUV<yuvBits>::Type* dstV) {
  using Quantizer = typename avif::img::spec::Quantizer<rgbBits, yuvBits, isFullRange>;
  using RGBSpec = typename avif::img::spec::RGB<rgbBits>;
  float const r = static_cast<float>(ir) / RGBSpec::max;
  float const g = static_cast<float>(ig) / RGBSpec::max;
  float const b = static_cast<float>(ib) / RGBSpec::max;

  Converter<matrixType> converter;
  if (isMonoYUV) {
    float y = 0;
    converter.calcYUV(r,g,b, &y, nullptr, nullptr);
    *dstY = Quantizer::quantizeLuma(y);
  } else {
    float y = 0;
    float u = 0;
    float v = 0;
    converter.calcYUV(r,g,b, &y, &u, &v);
    *dstY = Quantizer::quantizeLuma(y);
    *dstU = Quantizer::quantizeChroma(u);
    *dstV = Quantizer::quantizeChroma(v);
  }
}

template <MatrixType matrixType, size_t rgbBits, size_t yuvBits, bool isMonoYUV, bool isFullRange>
constexpr std::tuple<typename avif::img::spec::RGB<rgbBits>::Type, typename avif::img::spec::RGB<rgbBits>::Type, typename avif::img::spec::RGB<rgbBits>::Type> calcRGB(typename avif::img::spec::YUV<yuvBits>::Type const* srcY, typename avif::img::spec::YUV<yuvBits>::Type const* srcU, typename avif::img::spec::YUV<yuvBits>::Type const* srcV) {
  using avif::img::spec::clamp;
  using Quantizer = typename avif::img::spec::Quantizer<rgbBits, yuvBits, isFullRange>;
  using RGBSpec = typename avif::img::spec::RGB<rgbBits>;
  using YUVSpec = typename avif::img::spec::YUV<yuvBits>;

  using RGBType = typename RGBSpec::Type;

  auto const y = Quantizer::dequantizeLuma(*srcY);
  auto const u = isMonoYUV ? 0.0f : Quantizer::dequantizeChroma(*srcU);
  auto const v = isMonoYUV ? 0.0f : Quantizer::dequantizeChroma(*srcV);

  Converter<matrixType> converter;
  auto const [r, g, b] = converter.calcRGB(y, u, v);

  auto const ir = static_cast<RGBType>(clamp<int>(static_cast<int>(std::round(r * RGBSpec::max)), 0, RGBSpec::max));
  auto const ig = static_cast<RGBType>(clamp<int>(static_cast<int>(std::round(g * RGBSpec::max)), 0, RGBSpec::max));
  auto const ib = static_cast<RGBType>(clamp<int>(static_cast<int>(std::round(b * RGBSpec::max)), 0, RGBSpec::max));
  return std::make_tuple(ir, ig, ib);
}

// MonochromeYUV version
template <MatrixType matrixType, uint8_t rgbBits, uint8_t yuvBits, bool fromMonoRGB, bool isFullRange>
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
        calcYUV<matrixType, rgbBits, yuvBits, true, isFullRange>(mono, mono, mono, &ptrY[x], nullptr, nullptr);
      } else {
        uint16_t const r = reinterpret_cast<RGBType const *>(ptr)[0];
        uint16_t const g = reinterpret_cast<RGBType const *>(ptr)[1];
        uint16_t const b = reinterpret_cast<RGBType const *>(ptr)[2];
        calcYUV<matrixType, rgbBits, yuvBits, true, isFullRange>(r, g, b, &ptrY[x], nullptr, nullptr);
      }
      ptr += bytesPerPixel;
    }
    lineY += strideY;
    line += stride;
  }
}

template <MatrixType matrixType, uint8_t rgbBits, uint8_t yuvBits, bool fromMonoRGB, bool isFullRange, bool subX, bool subY>
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
        calcYUV<matrixType, rgbBits, yuvBits, false, isFullRange>(mono, mono, mono, &ptrY[x], sampler.pixelInLine(ptrU, x), sampler.pixelInLine(ptrV, x));
      } else {
        uint16_t const r = reinterpret_cast<RGBType const *>(ptr)[0];
        uint16_t const g = reinterpret_cast<RGBType const *>(ptr)[1];
        uint16_t const b = reinterpret_cast<RGBType const *>(ptr)[2];
        calcYUV<matrixType, rgbBits, yuvBits, false, isFullRange>(r, g, b, &ptrY[x], sampler.pixelInLine(ptrU, x), sampler.pixelInLine(ptrV, x));
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
template <MatrixType matrixType, uint8_t rgbBits, uint8_t yuvBits, bool toMonoRGB, bool isFullRange>
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
        std::tie(mono, mono, mono) = calcRGB<matrixType, rgbBits, yuvBits, true, isFullRange>(&ptrY[x], nullptr, nullptr);
      } else {
        RGBType& r = reinterpret_cast<RGBType*>(ptr)[0];
        RGBType& g = reinterpret_cast<RGBType*>(ptr)[1];
        RGBType& b = reinterpret_cast<RGBType*>(ptr)[2];
        std::tie(r,g,b) = calcRGB<matrixType, rgbBits, yuvBits, true, isFullRange>(&ptrY[x], nullptr, nullptr);
      }
      ptr += bytesPerPixel;
    }
    lineY += strideY;
    line += stride;
  }
}

template <MatrixType matrixType, uint8_t rgbBits, uint8_t yuvBits, bool toMonoRGB, bool isFullRange, bool subX, bool subY>
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
        std::tie(mono, mono, mono) = calcRGB<matrixType, rgbBits, yuvBits, false, isFullRange>(&ptrY[x], sampler.pixelInLine(ptrU, x), sampler.pixelInLine(ptrV, x));
      } else {
        RGBType& r = reinterpret_cast<RGBType*>(ptr)[0];
        RGBType& g = reinterpret_cast<RGBType*>(ptr)[1];
        RGBType& b = reinterpret_cast<RGBType*>(ptr)[2];
        std::tie(r,g,b) = calcRGB<matrixType, rgbBits, yuvBits, false, isFullRange>(&ptrY[x], sampler.pixelInLine(ptrU, x), sampler.pixelInLine(ptrV, x));
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

template <MatrixType matrixType, uint8_t rgbBits, uint8_t yuvBits, bool toMonoRGB, bool isFullRange>
struct FromRGB final {
  void toI400(Image<rgbBits>& src, uint8_t* dstY, size_t strideY) {
    detail::convertFromRGB<matrixType, rgbBits, yuvBits, toMonoRGB, isFullRange>(src.width(), src.height(), src.bytesPerPixel(), src.data(), src.stride(), dstY, strideY);
  }
  void toI444(Image<rgbBits> const& src, uint8_t* dstY, size_t strideY, uint8_t* dstU, size_t strideU, uint8_t* dstV, size_t strideV) {
    detail::convertFromRGB<matrixType, rgbBits, yuvBits, toMonoRGB, isFullRange, false, false>(src.width(), src.height(), src.bytesPerPixel(), src.data(), src.stride(), dstY, strideY, dstU, strideU, dstV, strideV);
  }
  void toI422(Image<rgbBits> const& src, uint8_t* dstY, size_t strideY, uint8_t* dstU, size_t strideU, uint8_t* dstV, size_t strideV){
    detail::convertFromRGB<matrixType, rgbBits, yuvBits, toMonoRGB, isFullRange, true, false>(src.width(), src.height(), src.bytesPerPixel(), src.data(), src.stride(), dstY, strideY, dstU, strideU, dstV, strideV);
  }
  void toI420(Image<rgbBits> const& src, uint8_t* dstY, size_t strideY, uint8_t* dstU, size_t strideU, uint8_t* dstV, size_t strideV){
    detail::convertFromRGB<matrixType, rgbBits, yuvBits, toMonoRGB, isFullRange, true, true>(src.width(), src.height(), src.bytesPerPixel(), src.data(), src.stride(), dstY, strideY, dstU, strideU, dstV, strideV);
  }
};

template <MatrixType matrixType, uint8_t rgbBits, uint8_t yuvBits, bool isFullRange>
struct FromAlpha final {
  void toI400(Image<rgbBits>& src, uint8_t* dstY, size_t strideY) {
    switch(src.pixelOrder()) {
      case avif::img::PixelOrder::MonoA:
      case avif::img::PixelOrder::RGBA:
        detail::convertFromRGB<matrixType, rgbBits, yuvBits, true, isFullRange>(src.width(), src.height(), src.bytesPerPixel(), src.data() + (src.numComponents() - 1) * src.bytesPerComponent(), src.stride(), dstY, strideY);
        break;
      case avif::img::PixelOrder::Mono:
        throw std::domain_error("Cannot separate Alpha from Mono image.");
      case avif::img::PixelOrder::RGB:
        throw std::domain_error("Cannot separate Alpha from RGB image.");
    }
  }
};

template <MatrixType matrixType, uint8_t rgbBits, uint8_t yuvBits, bool toMonoRGB, bool isFullRange>
struct ToRGB final {
  void fromI400(Image<rgbBits>& dst, uint8_t* srcY, size_t strideY) {
    detail::convertFromYUV<matrixType, rgbBits, yuvBits, toMonoRGB, isFullRange>(dst.width(), dst.height(), dst.bytesPerPixel(), dst.data(), dst.stride(), srcY, strideY);
  }
  void fromI444(Image<rgbBits>& dst, uint8_t* srcY, size_t strideY, uint8_t* srcU, size_t strideU, uint8_t* srcV, size_t strideV) {
    detail::convertFromYUV<matrixType, rgbBits, yuvBits, toMonoRGB, isFullRange, false, false>(dst.width(), dst.height(), dst.bytesPerPixel(), dst.data(), dst.stride(), srcY, strideY, srcU, strideU, srcV, strideV);
  }
  void fromI422(Image<rgbBits>& dst, uint8_t* srcY, size_t strideY, uint8_t* srcU, size_t strideU, uint8_t* srcV, size_t strideV){
    detail::convertFromYUV<matrixType, rgbBits, yuvBits, toMonoRGB, isFullRange, true, false>(dst.width(), dst.height(), dst.bytesPerPixel(), dst.data(), dst.stride(), srcY, strideY, srcU, strideU, srcV, strideV);
  }
  void fromI420(Image<rgbBits>& dst, uint8_t* srcY, size_t strideY, uint8_t* srcU, size_t strideU, uint8_t* srcV, size_t strideV){
    detail::convertFromYUV<matrixType, rgbBits, yuvBits, toMonoRGB, isFullRange, true, true>(dst.width(), dst.height(), dst.bytesPerPixel(), dst.data(), dst.stride(), srcY, strideY, srcU, strideU, srcV, strideV);
  }
};
template <MatrixType matrixType, uint8_t rgbBits, uint8_t yuvBits, bool isFullRange>
struct ToAlpha final {
  void fromI400(Image<rgbBits>& dst, uint8_t* srcY, size_t strideY) {
    switch(dst.pixelOrder()) {
      case avif::img::PixelOrder::MonoA:
      case avif::img::PixelOrder::RGBA:
        detail::convertFromYUV<matrixType, rgbBits, yuvBits, true, isFullRange>(dst.width(), dst.height(), dst.bytesPerPixel(), dst.data() + (dst.numComponents() - 1) * dst.bytesPerComponent(), dst.stride(), srcY, strideY);
        break;
      case avif::img::PixelOrder::Mono:
        throw std::domain_error("Cannot store Alpha to Mono image.");
      case avif::img::PixelOrder::RGB:
        throw std::domain_error("Cannot store Alpha to RGB image.");
    }
  }
};

}
