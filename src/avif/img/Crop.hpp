//
// Created by psi on 2020/01/25.
//

#pragma once

#include <cassert>
#include <cmath>
#include "Image.hpp"
#include "../CleanApertureBox.hpp"

namespace avif::img {

template <size_t BitsPerComponent>
Image<BitsPerComponent> crop(Image<BitsPerComponent> const& src, CleanApertureBox const& clap) {
  float const horizOff = static_cast<float>(clap.horizOffN)/static_cast<float>(clap.horizOffD);
  float const vertOff = static_cast<float>(clap.vertOffN)/static_cast<float>(clap.vertOffD);

  float const pcX = horizOff + (static_cast<float>(src.width()) - 1)/2;
  float const pcY = vertOff + (static_cast<float>(src.height()) - 1)/2;

  float const cleanApertureWidth = static_cast<float>(clap.cleanApertureWidthN)/static_cast<float>(clap.cleanApertureWidthD);
  float const cleanApertureHeight = static_cast<float>(clap.cleanApertureHeightN)/static_cast<float>(clap.cleanApertureHeightD);

  auto const offX = std::max(static_cast<size_t>(std::round(pcX - (cleanApertureWidth - 1) / 2)), static_cast<size_t>(0));
  auto const offY = std::max(static_cast<size_t>(std::round(pcY - (cleanApertureHeight - 1) / 2)), static_cast<size_t>(0));

  size_t const width = std::min(static_cast<size_t>(std::round(cleanApertureWidth)), src.width() - offX);
  size_t const height = std::min(static_cast<size_t>(std::round(cleanApertureHeight)), src.height() - offY);

  Image<BitsPerComponent> dst = Image<BitsPerComponent>::createEmptyImage(src.pixelOrder(), width, height);

  // src
  size_t const srcStride = src.stride();
  uint8_t const* srcLine = src.data() + (srcStride * offY);

  // dst
  uint8_t* dstLine = dst.data();
  size_t const dstStride = dst.stride();
  size_t const dstHeight = dst.height();

  // common
  size_t const bytesPerPixel = src.bytesPerPixel();
  size_t lineCopySize = width * bytesPerPixel;
  size_t lineOffset = offX * bytesPerPixel;
  size_t lineEnd = lineOffset + lineCopySize;

  for (size_t y = 0; y < dstHeight; ++y) {
    std::copy(srcLine + lineOffset, srcLine + lineEnd, dstLine);
    srcLine += srcStride;
    dstLine += dstStride;
  }
  return dst;
}

}