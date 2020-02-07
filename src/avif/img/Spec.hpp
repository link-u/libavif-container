//
// Created by psi on 2020/02/07.
//

#pragma once

#include <cstdint>
#include <cstddef>
#include <cmath>

// https://www.itu.int/dms_pubrec/itu-r/rec/bt/R-REC-BT.2020-2-201510-I!!PDF-E.pdf
// https://www.itu.int/dms_pubrec/itu-r/rec/bt/R-REC-BT.709-6-201506-I!!PDF-E.pdf

namespace avif::img::spec {

template <typename T>
static T clamp(T value, T low, T high) {
  return value < low ? low : (value > high ? high : value);
}

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

template <uint8_t bitsPerComponent> struct RGB;
template <> struct RGB<8> {
  using Type = uint8_t;
  static constexpr float max = 255.0f;
  static constexpr size_t bytesPerComponent = sizeof(uint8_t);
};
template <> struct RGB<16> {
  using Type = uint16_t;
  static constexpr float max = 65535.0f;
  static constexpr size_t bytesPerComponent = sizeof(uint16_t);
};

template <size_t rgbBits, size_t yuvBits, bool isFullRange>
struct Quantizer final{};

// https://www.itu.int/rec/T-REC-H.273-201612-I/en
template <size_t rgbBits, size_t yuvBits>
struct Quantizer<rgbBits, yuvBits, true> final{
  using YUVSpec = typename spec::YUV<yuvBits>;
  using YUVType = typename YUVSpec::Type;
  constexpr static YUVType quantizeLuma(float const luma) {
    // https://www.itu.int/rec/T-REC-H.273-201612-I/en
    // 8.3 Matrix coefficients
    // Equation (29)
    int constexpr maxYUV = (1u << yuvBits) - 1u;
    return static_cast<YUVType>(clamp<int>(static_cast<int>(std::round(luma * maxYUV)), 0, maxYUV));
  }
  constexpr static float dequantizeLuma(YUVType const luma) {
    // FIXME(ledyba): H.273 does not specify about dequantization
    auto constexpr maxYUV = static_cast<float>((1u << yuvBits) - 1u);
    return static_cast<float>(luma) / static_cast<float>(maxYUV);
  }
  constexpr static YUVType quantizeChroma(float const chroma) {
    // https://www.itu.int/rec/T-REC-H.273-201612-I/en
    // 8.3 Matrix coefficients
    // Equation (30, 31)
    int constexpr maxYUV = (1u << yuvBits) - 1u;
    int constexpr bias = YUVSpec::bias;
    return static_cast<YUVType>(clamp<int>(static_cast<int>(std::round(chroma * maxYUV) + bias), 0, maxYUV));
  }
  constexpr static float dequantizeChroma(YUVType const chroma) {
    // FIXME(ledyba): H.273 does not specify about dequantization
    int constexpr maxYUV = (1u << yuvBits) - 1u;
    int constexpr bias = YUVSpec::bias;
    return clamp<float>((static_cast<float>(chroma) - bias) / static_cast<float>(maxYUV), -0.5, 0.5);
  }
};

template <size_t rgbBits, size_t yuvBits>
struct Quantizer<rgbBits, yuvBits, false> final{
  using YUVSpec = typename spec::YUV<yuvBits>;
  using YUVType = typename YUVSpec::Type;
  static YUVType quantizeLuma(float const luma) {
    // https://www.itu.int/rec/T-REC-H.273-201612-I/en
    // 8.3 Matrix coefficients
    // Equation (23)
    int constexpr maxYUV = (1u << yuvBits) - 1u;
    auto constexpr shift = static_cast<float>(1u << (yuvBits - 8u));
    return static_cast<YUVType>(clamp<int>(static_cast<int>(std::round((luma * 219 + 16) * shift)), 0, maxYUV));
  }
  constexpr static float dequantizeLuma(YUVType const luma) {
    // FIXME(ledyba): H.273 does not specify about dequantization
    auto constexpr maxYUV = static_cast<float>((1u << yuvBits) - 1u);
    auto constexpr shift = static_cast<float>(1u << (yuvBits - 8u));
    return ((static_cast<float>(luma) / shift) - 16.0f) / 219.0f;
  }
  static YUVType quantizeChroma(float const chroma) {
    // https://www.itu.int/rec/T-REC-H.273-201612-I/en
    // 8.3 Matrix coefficients
    // Equation (24, 25)
    int constexpr maxYUV = (1u << yuvBits) - 1u;
    int constexpr shift = static_cast<float>(1u << (yuvBits - 8u));
    return static_cast<YUVType>(clamp<int>(static_cast<int>(std::round((chroma * 224 + 128) * shift)), 0, maxYUV));
  }
  constexpr static float dequantizeChroma(YUVType const chroma) {
    // FIXME(ledyba): H.273 does not specify about dequantization
    int constexpr maxYUV = (1u << yuvBits) - 1u;
    float constexpr bias = YUVSpec::bias;
    auto constexpr shift = static_cast<float>(1u << (yuvBits - 8u));
    return ((static_cast<float>(chroma) / shift) - 128.0f) / 224.0f;
  }
};

}