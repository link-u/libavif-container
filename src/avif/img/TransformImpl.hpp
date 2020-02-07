//
// Created by psi on 2020/01/25.
//

#pragma once

#include <tuple>
#include "Image.hpp"
#include "../ImageMirrorBox.hpp"
#include "../ImageRotationBox.hpp"

namespace avif::img::transform {

template <ImageMirrorBox::Axis>
 struct FlipTrans;
template <>
struct FlipTrans<ImageMirrorBox::Axis::Vertical> {
  std::tuple <size_t, size_t> operator()(size_t srcWidth, size_t x, size_t srcHeight, size_t y) {
    return std::make_tuple(x, srcHeight - y - 1);
  }
};

template <>
struct FlipTrans<ImageMirrorBox::Axis::Horizontal> {
  std::tuple <size_t,size_t> operator()(size_t srcWidth, size_t x, size_t srcHeight, size_t y) {
    return std::make_tuple(srcWidth - x - 1, y);
  }
};

template <ImageRotationBox::Rotation>
struct RotateTrans;
template <>
struct RotateTrans<ImageRotationBox::Rotation::Rot0> {
  std::tuple <size_t, size_t> operator()(size_t srcWidth, size_t x, size_t srcHeight, size_t y) {
    return std::make_tuple(x, y);
  }
};

template <>
struct RotateTrans<ImageRotationBox::Rotation::Rot90> {
  std::tuple <size_t, size_t> operator()(size_t srcWidth, size_t x, size_t srcHeight, size_t y) {
    return std::make_tuple(srcWidth - y - 1, x);
  }
};

template <>
struct RotateTrans<ImageRotationBox::Rotation::Rot180> {
  std::tuple <size_t, size_t> operator()(size_t srcWidth, size_t x, size_t srcHeight, size_t y) {
    return std::make_tuple(srcWidth - x - 1, srcHeight - y - 1);
  }
};

template <>
struct RotateTrans<ImageRotationBox::Rotation::Rot270> {
  std::tuple <size_t, size_t> operator()(size_t srcWidth, size_t x, size_t srcHeight, size_t y) {
    return std::make_tuple(y, srcHeight - x - 1);
  }
};

template < size_t BitsPerComponent, typename Trans >
void fill(Image<BitsPerComponent> const& src, Image<BitsPerComponent>& dst) {
  Trans trans;

  using PixelType = typename spec::RGB <BitsPerComponent>::Type;

  // src
  uint8_t const* srcBase = src.data();
  size_t const srcStride = src.stride();
  size_t const srcBytesPerPixel = dst.bytesPerPixel();
  size_t const srcWidth = src.width();
  size_t const srcHeight = src.height();

  // dst
  uint8_t* dstLine = dst.data();
  size_t const dstStride = dst.stride();
  size_t const dstBytesPerPixel = dst.bytesPerPixel();
  size_t const dstWidth = dst.width();
  size_t const dstHeight = dst.height();

  // common
  size_t const components = src.numComponents();
  for (size_t y = 0; y < dstHeight; ++y) {
    auto * dstPtr = reinterpret_cast<PixelType*>(dstLine);
    for (size_t x = 0; x < dstWidth; ++x) {
      auto * dstPixel = reinterpret_cast<PixelType*>(dstPtr);
      auto from = trans(srcWidth, x, srcHeight, y);
      auto const * srcPixel = reinterpret_cast<PixelType const*>(srcBase + (srcStride * std::get<1>(from)) + (srcBytesPerPixel * std::get<0>(from)));
      for (size_t c = 0; c < components; ++c) {
        dstPixel[c] = srcPixel[c];
      }
      dstPtr += dstBytesPerPixel;
    }
    dstLine += dstStride;
  }
}

}
