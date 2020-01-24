//
// Created by psi on 2020/01/25.
//

#pragma once

#include <cassert>
#include "Image.hpp"
#include "../ImageMirrorBox.hpp"
#include "../ImageRotationBox.hpp"

namespace avif::img {
namespace transform {

template <ImageMirrorBox::Axis>
 struct FlipTrans;
template <>
struct FlipTrans<ImageMirrorBox::Axis::Vertical> {
  std::tuple <size_t, size_t> operator()(size_t srcWidth, size_t x, size_t srcHeight, size_t y) {
    return std::make_tuple(x, srcHeight - y);
  }
};

template <>
struct FlipTrans<ImageMirrorBox::Axis::Horizontal> {
  std::tuple <size_t,size_t> operator()(size_t srcWidth, size_t x, size_t srcHeight, size_t y) {
    return std::make_tuple(srcWidth - x, y);
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
    return std::make_tuple(y, x);
  }
};

template <>
struct RotateTrans<ImageRotationBox::Rotation::Rot180> {
  std::tuple <size_t, size_t> operator()(size_t srcWidth, size_t x, size_t srcHeight, size_t y) {
    return std::make_tuple(srcWidth - x, srcHeight - y);
  }
};

template <>
struct RotateTrans<ImageRotationBox::Rotation::Rot270> {
  std::tuple <size_t, size_t> operator()(size_t srcWidth, size_t x, size_t srcHeight, size_t y) {
    return std::make_tuple(y, srcHeight - x);
  }
};

template < size_t BitsPerComponent, typename Trans >
void fill(Image <BitsPerComponent> const& src, Image <BitsPerComponent>& dst) {
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

template <size_t BitsPerComponent>
Image <BitsPerComponent> flip(Image <BitsPerComponent> const & src, ImageMirrorBox::Axis const axis) {
  Image<BitsPerComponent> dst = Image <BitsPerComponent> ::createEmptyImage(src.pixelOrder(), src.width(), src.height());
  switch (axis) {
  case ImageMirrorBox::Axis::Horizontal:
    transform::fill<BitsPerComponent, transform::FlipTrans<ImageMirrorBox::Axis::Horizontal>>(src, dst);
    break;
  case ImageMirrorBox::Axis::Vertical:
    transform::fill<BitsPerComponent, transform::FlipTrans<ImageMirrorBox::Axis::Vertical>>(src, dst);
    break;
  default:
    assert("Do not come here" && (axis == ImageMirrorBox::Axis::Horizontal || axis == ImageMirrorBox::Axis::Vertical));
    break;
  }
  return std::move(dst);
}

template <size_t BitsPerComponent>
Image <BitsPerComponent> rotate(Image < BitsPerComponent > const & src, ImageRotationBox::Rotation const rotation) {
  Image<BitsPerComponent> dst;
  switch (rotation) {
  case ImageRotationBox::Rotation::Rot0: {
    dst = Image<BitsPerComponent>::createEmptyImage(src.pixelOrder(), src.width(), src.height());
    transform::fill<BitsPerComponent, transform::RotateTrans<ImageRotationBox::Rotation::Rot0>>(src, dst);
    break;
  }
  case ImageRotationBox::Rotation::Rot90: {
    dst = Image<BitsPerComponent>::createEmptyImage(src.pixelOrder(), src.height(), src.width());
    transform::fill<BitsPerComponent, transform::RotateTrans<ImageRotationBox::Rotation::Rot90>>(src, dst);
    break;
  }
  case ImageRotationBox::Rotation::Rot180: {
    dst = Image<BitsPerComponent>::createEmptyImage(src.pixelOrder(), src.width(), src.height());
    transform::fill<BitsPerComponent, transform::RotateTrans<ImageRotationBox::Rotation::Rot180>>(src, dst);
    break;
  }
  case ImageRotationBox::Rotation::Rot270: {
    dst = Image<BitsPerComponent>::createEmptyImage(src.pixelOrder(), src.height(), src.width());
    transform::fill<BitsPerComponent, transform::RotateTrans<ImageRotationBox::Rotation::Rot270>>(src, dst);
    break;
  }
  default:
    assert("Do not come here" && (rotation == ImageRotationBox::Rotation::Rot0 || rotation == ImageRotationBox::Rotation::Rot90 || rotation == ImageRotationBox::Rotation::Rot180 || rotation == ImageRotationBox::Rotation::Rot270));
    break;
  }
  return std::move(dst);
}

}