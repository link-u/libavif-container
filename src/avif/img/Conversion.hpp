//
// Created by psi on 2020/01/21.
//

#pragma once

#include <cstdint>
#include <cmath>
#include <tuple>
#include <fmt/format.h>
#include "./color/Matrix.hpp"
#include "./color/Math.hpp"
#include "Image.hpp"

namespace avif::img {

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

template <typename ConverterFactory, size_t rgbBits, size_t yuvBits, bool isMonoYUV, bool isFullRange>
constexpr void calcYUV(uint16_t const ir, uint16_t const ig, uint16_t const ib, typename avif::img::color::YUV<yuvBits>::Type* dstY, typename avif::img::color::YUV<yuvBits>::Type* dstU, typename avif::img::color::YUV<yuvBits>::Type* dstV) {
  using Quantizer = typename avif::img::color::Quantizer<rgbBits, yuvBits, isFullRange>;
  using RGBSpec = typename avif::img::color::RGB<rgbBits>;
  auto converter = ConverterFactory::create();
  float const r = static_cast<float>(ir) / RGBSpec::max;
  float const g = static_cast<float>(ig) / RGBSpec::max;
  float const b = static_cast<float>(ib) / RGBSpec::max;

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

template <typename ConverterFactory, size_t rgbBits, size_t yuvBits, bool isMonoYUV, bool isFullRange>
constexpr std::tuple<typename avif::img::color::RGB<rgbBits>::Type, typename avif::img::color::RGB<rgbBits>::Type, typename avif::img::color::RGB<rgbBits>::Type> calcRGB(typename avif::img::color::YUV<yuvBits>::Type const* srcY, typename avif::img::color::YUV<yuvBits>::Type const* srcU, typename avif::img::color::YUV<yuvBits>::Type const* srcV) {
  using avif::img::color::clamp;
  using Quantizer = typename avif::img::color::Quantizer<rgbBits, yuvBits, isFullRange>;
  using RGBSpec = typename avif::img::color::RGB<rgbBits>;
  using YUVSpec = typename avif::img::color::YUV<yuvBits>;
  auto converter = ConverterFactory::create();

  using RGBType = typename RGBSpec::Type;

  auto const y = Quantizer::dequantizeLuma(*srcY);
  auto const u = isMonoYUV ? 0.0f : Quantizer::dequantizeChroma(*srcU);
  auto const v = isMonoYUV ? 0.0f : Quantizer::dequantizeChroma(*srcV);

  auto const [r, g, b] = converter.calcRGB(y, u, v);

  auto const ir = static_cast<RGBType>(clamp<int>(static_cast<int>(std::round(r * RGBSpec::max)), 0, RGBSpec::max));
  auto const ig = static_cast<RGBType>(clamp<int>(static_cast<int>(std::round(g * RGBSpec::max)), 0, RGBSpec::max));
  auto const ib = static_cast<RGBType>(clamp<int>(static_cast<int>(std::round(b * RGBSpec::max)), 0, RGBSpec::max));
  return std::make_tuple(ir, ig, ib);
}

// MonochromeYUV version
template <typename ConverterFactory, uint8_t rgbBits, uint8_t yuvBits, bool fromMonoRGB, bool isFullRange>
void constexpr convertFromRGB(size_t width, size_t height, uint8_t bytesPerPixel, uint8_t const* src, size_t const stride, uint8_t* const dstY, size_t const strideY) {
  using avif::img::color::clamp;
  using RGBSpec = typename avif::img::color::RGB<rgbBits>;
  using YUVSpec = typename avif::img::color::YUV<yuvBits>;

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
        calcYUV<ConverterFactory, rgbBits, yuvBits, true, isFullRange>(mono, mono, mono, &ptrY[x], nullptr, nullptr);
      } else {
        uint16_t const r = reinterpret_cast<RGBType const *>(ptr)[0];
        uint16_t const g = reinterpret_cast<RGBType const *>(ptr)[1];
        uint16_t const b = reinterpret_cast<RGBType const *>(ptr)[2];
        calcYUV<ConverterFactory, rgbBits, yuvBits, true, isFullRange>(r, g, b, &ptrY[x], nullptr, nullptr);
      }
      ptr += bytesPerPixel;
    }
    lineY += strideY;
    line += stride;
  }
}

template <typename ConverterFactory, uint8_t rgbBits, uint8_t yuvBits, bool fromMonoRGB, bool isFullRange, bool subX, bool subY>
void constexpr convertFromRGB(size_t width, size_t height, uint8_t bytesPerPixel, uint8_t const* src, size_t const stride, uint8_t* const dstY, size_t const strideY, uint8_t* const dstU, size_t const strideU, uint8_t* const dstV, size_t const strideV) {
  using avif::img::color::clamp;
  using RGBSpec = typename avif::img::color::RGB<rgbBits>;
  using YUVSpec = typename avif::img::color::YUV<yuvBits>;

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
        calcYUV<ConverterFactory, rgbBits, yuvBits, false, isFullRange>(mono, mono, mono, &ptrY[x], sampler.pixelInLine(ptrU, x), sampler.pixelInLine(ptrV, x));
      } else {
        uint16_t const r = reinterpret_cast<RGBType const *>(ptr)[0];
        uint16_t const g = reinterpret_cast<RGBType const *>(ptr)[1];
        uint16_t const b = reinterpret_cast<RGBType const *>(ptr)[2];
        calcYUV<ConverterFactory, rgbBits, yuvBits, false, isFullRange>(r, g, b, &ptrY[x], sampler.pixelInLine(ptrU, x), sampler.pixelInLine(ptrV, x));
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
template <typename ConverterFactory, uint8_t rgbBits, uint8_t yuvBits, bool toMonoRGB, bool isFullRange>
void constexpr convertFromYUV(size_t width, size_t height, uint8_t bytesPerPixel, uint8_t* dst, size_t stride, uint8_t const* srcY, size_t strideY) {
  using RGBSpec = typename avif::img::color::RGB<rgbBits>;
  using YUVSpec = typename avif::img::color::YUV<yuvBits>;

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
        std::tie(mono, mono, mono) = calcRGB<ConverterFactory, rgbBits, yuvBits, true, isFullRange>(&ptrY[x], nullptr, nullptr);
      } else {
        RGBType& r = reinterpret_cast<RGBType*>(ptr)[0];
        RGBType& g = reinterpret_cast<RGBType*>(ptr)[1];
        RGBType& b = reinterpret_cast<RGBType*>(ptr)[2];
        std::tie(r,g,b) = calcRGB<ConverterFactory, rgbBits, yuvBits, true, isFullRange>(&ptrY[x], nullptr, nullptr);
      }
      ptr += bytesPerPixel;
    }
    lineY += strideY;
    line += stride;
  }
}

template <typename ConverterFactory, uint8_t rgbBits, uint8_t yuvBits, bool toMonoRGB, bool isFullRange, bool subX, bool subY>
void constexpr convertFromYUV(size_t width, size_t height, uint8_t bytesPerPixel, uint8_t* dst, size_t stride, uint8_t const* srcY, size_t strideY, uint8_t const* srcU, size_t strideU, uint8_t const* srcV, size_t strideV) {
  using RGBSpec = typename avif::img::color::RGB<rgbBits>;
  using YUVSpec = typename avif::img::color::YUV<yuvBits>;

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
        std::tie(mono, mono, mono) = calcRGB<ConverterFactory, rgbBits, yuvBits, false, isFullRange>(&ptrY[x], sampler.pixelInLine(ptrU, x), sampler.pixelInLine(ptrV, x));
      } else {
        RGBType& r = reinterpret_cast<RGBType*>(ptr)[0];
        RGBType& g = reinterpret_cast<RGBType*>(ptr)[1];
        RGBType& b = reinterpret_cast<RGBType*>(ptr)[2];
        std::tie(r,g,b) = calcRGB<ConverterFactory, rgbBits, yuvBits, false, isFullRange>(&ptrY[x], sampler.pixelInLine(ptrU, x), sampler.pixelInLine(ptrV, x));
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

template <typename ConverterFactory, uint8_t rgbBits, uint8_t yuvBits, bool toMonoRGB, bool isFullRange>
struct FromRGB final {
  static void toI400(Image<rgbBits>& src, uint8_t* dstY, size_t strideY) {
    detail::convertFromRGB<ConverterFactory, rgbBits, yuvBits, toMonoRGB, isFullRange>(src.width(), src.height(), src.bytesPerPixel(), src.data(), src.stride(), dstY, strideY);
  }
  static void toI444(Image<rgbBits> const& src, uint8_t* dstY, size_t strideY, uint8_t* dstU, size_t strideU, uint8_t* dstV, size_t strideV) {
    detail::convertFromRGB<ConverterFactory, rgbBits, yuvBits, toMonoRGB, isFullRange, false, false>(src.width(), src.height(), src.bytesPerPixel(), src.data(), src.stride(), dstY, strideY, dstU, strideU, dstV, strideV);
  }
  static void toI422(Image<rgbBits> const& src, uint8_t* dstY, size_t strideY, uint8_t* dstU, size_t strideU, uint8_t* dstV, size_t strideV){
    detail::convertFromRGB<ConverterFactory, rgbBits, yuvBits, toMonoRGB, isFullRange, true, false>(src.width(), src.height(), src.bytesPerPixel(), src.data(), src.stride(), dstY, strideY, dstU, strideU, dstV, strideV);
  }
  static void toI420(Image<rgbBits> const& src, uint8_t* dstY, size_t strideY, uint8_t* dstU, size_t strideU, uint8_t* dstV, size_t strideV){
    detail::convertFromRGB<ConverterFactory, rgbBits, yuvBits, toMonoRGB, isFullRange, true, true>(src.width(), src.height(), src.bytesPerPixel(), src.data(), src.stride(), dstY, strideY, dstU, strideU, dstV, strideV);
  }
};

template <typename ConverterFactory, uint8_t rgbBits, uint8_t yuvBits, bool isFullRange>
struct FromAlpha final {
  static void toI400(Image<rgbBits>& src, uint8_t* dstY, size_t strideY) {
    switch(src.pixelOrder()) {
      case avif::img::PixelOrder::MonoA:
      case avif::img::PixelOrder::RGBA:
        detail::convertFromRGB<ConverterFactory, rgbBits, yuvBits, true, isFullRange>(src.width(), src.height(), src.bytesPerPixel(), src.data() + (src.numComponents() - 1) * src.bytesPerComponent(), src.stride(), dstY, strideY);
        break;
      case avif::img::PixelOrder::Mono:
        throw std::domain_error("Cannot separate Alpha from Mono image.");
      case avif::img::PixelOrder::RGB:
        throw std::domain_error("Cannot separate Alpha from RGB image.");
    }
  }
};

template <typename ConverterFactory, uint8_t rgbBits, uint8_t yuvBits, bool toMonoRGB, bool isFullRange>
struct ToRGB final {
  static void fromI400(Image<rgbBits>& dst, uint8_t* srcY, size_t strideY) {
    detail::convertFromYUV<ConverterFactory, rgbBits, yuvBits, toMonoRGB, isFullRange>(dst.width(), dst.height(), dst.bytesPerPixel(), dst.data(), dst.stride(), srcY, strideY);
  }
  static void fromI444(Image<rgbBits>& dst, uint8_t* srcY, size_t strideY, uint8_t* srcU, size_t strideU, uint8_t* srcV, size_t strideV) {
    detail::convertFromYUV<ConverterFactory, rgbBits, yuvBits, toMonoRGB, isFullRange, false, false>(dst.width(), dst.height(), dst.bytesPerPixel(), dst.data(), dst.stride(), srcY, strideY, srcU, strideU, srcV, strideV);
  }
  static void fromI422(Image<rgbBits>& dst, uint8_t* srcY, size_t strideY, uint8_t* srcU, size_t strideU, uint8_t* srcV, size_t strideV){
    detail::convertFromYUV<ConverterFactory, rgbBits, yuvBits, toMonoRGB, isFullRange, true, false>(dst.width(), dst.height(), dst.bytesPerPixel(), dst.data(), dst.stride(), srcY, strideY, srcU, strideU, srcV, strideV);
  }
  static void fromI420(Image<rgbBits>& dst, uint8_t* srcY, size_t strideY, uint8_t* srcU, size_t strideU, uint8_t* srcV, size_t strideV){
    detail::convertFromYUV<ConverterFactory, rgbBits, yuvBits, toMonoRGB, isFullRange, true, true>(dst.width(), dst.height(), dst.bytesPerPixel(), dst.data(), dst.stride(), srcY, strideY, srcU, strideU, srcV, strideV);
  }
};
template <typename ConverterFactory, uint8_t rgbBits, uint8_t yuvBits, bool isFullRange>
struct ToAlpha final {
  static void fromI400(Image<rgbBits>& dst, uint8_t* srcY, size_t strideY) {
    switch(dst.pixelOrder()) {
      case avif::img::PixelOrder::MonoA:
      case avif::img::PixelOrder::RGBA:
        detail::convertFromYUV<ConverterFactory, rgbBits, yuvBits, true, isFullRange>(dst.width(), dst.height(), dst.bytesPerPixel(), dst.data() + (dst.numComponents() - 1) * dst.bytesPerComponent(), dst.stride(), srcY, strideY);
        break;
      case avif::img::PixelOrder::Mono:
        throw std::domain_error("Cannot store Alpha to Mono image.");
      case avif::img::PixelOrder::RGB:
        throw std::domain_error("Cannot store Alpha to RGB image.");
    }
  }
};

}
