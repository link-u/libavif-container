//
// Created by psi on 2020/01/21.
//

#pragma once

#include <cstdint>
#include <cmath>
#include <tuple>
#include "Image.hpp"

namespace avif::img {

// https://www.itu.int/dms_pubrec/itu-r/rec/bt/R-REC-BT.2020-2-201510-I!!PDF-E.pdf
// https://www.itu.int/dms_pubrec/itu-r/rec/bt/R-REC-BT.709-6-201506-I!!PDF-E.pdf

namespace spec {

template <uint8_t bitsPerComponent> struct YUV;
template <> struct YUV<8> {
  using Type = uint8_t;
  static constexpr uint8_t bias = 128;
};
template <> struct YUV<10> {
  using Type = uint16_t;
  static constexpr uint16_t bias = 512;
};
template <> struct YUV<12> {
  using Type = uint16_t;
  static constexpr uint16_t bias = 2048;
};


template <typename T, bool subX, bool subY>
struct ChromaSampler {
  static_assert(!std::is_pointer<T>::value);
  static constexpr bool isConst = std::is_const<T>::value;
  using LineType = typename std::conditional<isConst, const uint8_t, uint8_t>::type;
  constexpr LineType* nextLine(LineType* const currentLine, size_t const stride, size_t const y) {
    return currentLine == nullptr ? nullptr :
           subY                   ? (y % 2 == 1 ? (currentLine + stride) : currentLine) :
                                    (currentLine + stride);
  }
  constexpr T* pixelInLine(T* currentLine, size_t const x) {
    return currentLine == nullptr ? nullptr :
           subX                   ? &currentLine[x/2] :
                                    &currentLine[x];
  }
};

}

namespace detail{

template <typename T>
constexpr T clamp(T value, T low, T high) {
  return value < low ? low : (value > high ? high : value);
}

template <size_t rgbBits, size_t yuvBits>
constexpr void calcYUV(uint16_t const ir, uint16_t const ig, uint16_t const ib, typename spec::YUV<yuvBits>::Type* dstY, typename spec::YUV<yuvBits>::Type* dstU, typename spec::YUV<yuvBits>::Type* dstV) {
  using YUVType = typename spec::YUV<yuvBits>::Type;
  float const r = static_cast<float>(ir) / spec::RGB<rgbBits>::max;
  float const g = static_cast<float>(ig) / spec::RGB<rgbBits>::max;
  float const b = static_cast<float>(ib) / spec::RGB<rgbBits>::max;

  auto constexpr shift = static_cast<float>(1u << (yuvBits - 8u));
  int constexpr maxYUV = (1u << yuvBits) - 1u;
  float const y = 0.2627f * r + 0.6780f * g + 0.0593f * b;
  *dstY = static_cast<YUVType>(clamp(static_cast<int>(std::round(y * 219 + 16) * shift), 0, maxYUV));

  if (dstU) {
    float const u = (b - y) / 1.8814f;
    *dstU = static_cast<YUVType>(clamp(static_cast<int>(std::round(u * 224 + 128) * shift), 0, maxYUV));
  }
  if (dstV) {
    float const v = (r - y) / 1.4746f;
    *dstV = static_cast<YUVType>(clamp(static_cast<int>(std::round(v * 224 + 128) * shift), 0, maxYUV));
  }
}

template <size_t rgbBits, size_t yuvBits>
constexpr std::tuple<typename spec::RGB<rgbBits>::Type, typename spec::RGB<rgbBits>::Type, typename spec::RGB<rgbBits>::Type> calcRGB(typename spec::YUV<yuvBits>::Type const* srcY, typename spec::YUV<yuvBits>::Type const* srcU, typename spec::YUV<yuvBits>::Type const* srcV) {
  using RGBType = typename spec::RGB<rgbBits>::Type;

  auto constexpr shift = static_cast<float>(1u << (yuvBits - 8u));
  auto const y = ((static_cast<float>(*srcY) / shift) - 16.0f) / 219.0f;
  auto const u = ((static_cast<float>(srcU != nullptr ? (*srcU) : spec::YUV<yuvBits>::bias) / shift) - 128.0f) / 224.0f;
  auto const v = ((static_cast<float>(srcV != nullptr ? (*srcV) : spec::YUV<yuvBits>::bias) / shift) - 128.0f) / 224.0f;

  float const r = y                    + (+1.47460f) * v;
  float const g = y + (-0.16455f) * u  + (-0.57135f) * v;
  float const b = y + (+1.88140f) * u;

  auto const ir = static_cast<RGBType>(clamp<int>(static_cast<int>(std::round(r * spec::RGB<rgbBits>::max)), 0, spec::RGB<rgbBits>::max));
  auto const ig = static_cast<RGBType>(clamp<int>(static_cast<int>(std::round(g * spec::RGB<rgbBits>::max)), 0, spec::RGB<rgbBits>::max));
  auto const ib = static_cast<RGBType>(clamp<int>(static_cast<int>(std::round(b * spec::RGB<rgbBits>::max)), 0, spec::RGB<rgbBits>::max));
  return std::make_tuple(ir, ig, ib);
}

template <uint8_t rgbBits, uint8_t yuvBits, bool subX, bool subY>
void constexpr convertFromRGB(size_t width, size_t height, uint8_t bytesPerPixel, uint8_t const* src, size_t stride, uint8_t* dstY, size_t strideY, uint8_t* dstU, size_t strideU, uint8_t* dstV, size_t strideV) {
  using YUVType = typename spec::YUV<yuvBits>::Type;
  using RGBType = typename spec::RGB<rgbBits>::Type const;
  spec::ChromaSampler<YUVType, subX, subY> sampler;
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
      uint16_t const r = reinterpret_cast<RGBType const *>(ptr)[0];
      uint16_t const g = reinterpret_cast<RGBType const *>(ptr)[1];
      uint16_t const b = reinterpret_cast<RGBType const *>(ptr)[2];
      calcYUV<rgbBits, yuvBits>(r, g, b, &ptrY[x], sampler.pixelInLine(ptrU, x), sampler.pixelInLine(ptrV, x));
      ptr += bytesPerPixel;
    }
    lineY += strideY;
    lineU = sampler.nextLine(lineU, strideU, y);
    lineV = sampler.nextLine(lineV, strideV, y);
    line += stride;
  }
}

template <uint8_t rgbBits, uint8_t yuvBits, bool subX, bool subY>
void constexpr convertFromYUV(size_t width, size_t height, uint8_t bytesPerPixel, uint8_t* dst, size_t stride, uint8_t const* srcY, size_t strideY, uint8_t const* srcU, size_t strideU, uint8_t const* srcV, size_t strideV) {
  using YUVType = typename spec::YUV<yuvBits>::Type const;
  using RGBType = typename spec::RGB<rgbBits>::Type;
  spec::ChromaSampler<YUVType, subX, subY> sampler;
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
      RGBType& r = reinterpret_cast<RGBType*>(ptr)[0];
      RGBType& g = reinterpret_cast<RGBType*>(ptr)[1];
      RGBType& b = reinterpret_cast<RGBType*>(ptr)[2];
      std::tie(r,g,b) = calcRGB<rgbBits, yuvBits>(&ptrY[x], sampler.pixelInLine(ptrU, x), sampler.pixelInLine(ptrV, x));
      ptr += bytesPerPixel;
    }
    lineY += strideY;
    lineU = sampler.nextLine(lineU, strideU, y);
    lineV = sampler.nextLine(lineV, strideV, y);
    line += stride;
  }
}

}


template <uint8_t rgbBits, uint8_t yuvBits>
struct FromRGB final {
  void toI444(Image<rgbBits> const& src, uint8_t* dstY, size_t strideY, uint8_t* dstU, size_t strideU, uint8_t* dstV, size_t strideV) {
    detail::convertFromRGB<rgbBits, yuvBits, false, false>(src.width(), src.height(), src.bytesPerPixel(), src.data(), src.stride(), dstY, strideY, dstU, strideU, dstV, strideV);
  }
  void toI422(Image<rgbBits> const& src, uint8_t* dstY, size_t strideY, uint8_t* dstU, size_t strideU, uint8_t* dstV, size_t strideV){
    detail::convertFromRGB<rgbBits, yuvBits, true, false>(src.width(), src.height(), src.bytesPerPixel(), src.data(), src.stride(), dstY, strideY, dstU, strideU, dstV, strideV);
  }
  void toI420(Image<rgbBits> const& src, uint8_t* dstY, size_t strideY, uint8_t* dstU, size_t strideU, uint8_t* dstV, size_t strideV){
    detail::convertFromRGB<rgbBits, yuvBits, true, true>(src.width(), src.height(), src.bytesPerPixel(), src.data(), src.stride(), dstY, strideY, dstU, strideU, dstV, strideV);
  }
};

template <uint8_t rgbBits, uint8_t yuvBits>
struct ToRGB final {
  void fromI444(Image<rgbBits>& dst, uint8_t* srcY, size_t strideY, uint8_t* srcU, size_t strideU, uint8_t* srcV, size_t strideV) {
    detail::convertFromYUV<rgbBits, yuvBits, false, false>(dst.width(), dst.height(), dst.bytesPerPixel(), dst.data(), dst.stride(), srcY, strideY, srcU, strideU, srcV, strideV);
  }
  void fromI422(Image<rgbBits>& dst, uint8_t* srcY, size_t strideY, uint8_t* srcU, size_t strideU, uint8_t* srcV, size_t strideV){
    detail::convertFromYUV<rgbBits, yuvBits, true, false>(dst.width(), dst.height(), dst.bytesPerPixel(), dst.data(), dst.stride(), srcY, strideY, srcU, strideU, srcV, strideV);
  }
  void fromI420(Image<rgbBits>& dst, uint8_t* srcY, size_t strideY, uint8_t* srcU, size_t strideU, uint8_t* srcV, size_t strideV){
    detail::convertFromYUV<rgbBits, yuvBits, true, true>(dst.width(), dst.height(), dst.bytesPerPixel(), dst.data(), dst.stride(), srcY, strideY, srcU, strideU, srcV, strideV);
  }
};

}



