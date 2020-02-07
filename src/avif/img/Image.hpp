//
// Created by psi on 2020/01/05.
//

#pragma once

#include <vector>
#include <cstdint>
#include <cassert>

#include "Spec.hpp"

namespace avif::img {

enum class PixelOrder {
  RGB, /* [R,G,B], [R,G,B], ... */
  RGBA, /* [R,G,B,A], [R,G,B,A], ... */
};

template <size_t BitsPerComponent>
class Image final {
  static_assert(BitsPerComponent == 8 || BitsPerComponent == 16);
private:
  PixelOrder pixelOrder_{};
  uint32_t width_{};
  uint32_t height_{};
  uint32_t bytesPerPixel_{};
  uint32_t stride_{};
  std::vector<uint8_t> data_{};
public:
  Image() = default;
  Image(Image const&) = default;
  Image(Image&&) noexcept = default;
  Image& operator=(Image const&) = default;
  Image& operator=(Image&&) noexcept = default;
  ~Image() noexcept = default;
public:
  explicit Image(PixelOrder pixelOrder, uint32_t width, uint32_t height, uint32_t bytesPerPiexl, uint32_t stride, std::vector<uint8_t> data)
  :pixelOrder_(pixelOrder)
  ,width_(width)
  ,height_(height)
  ,bytesPerPixel_(bytesPerPiexl)
  ,stride_(stride)
  ,data_(std::move(data))
  {

  }
  static Image createEmptyImage(PixelOrder const pixelOrder, uint32_t const width, uint32_t const height) {
    std::vector<uint8_t> dstBuff;
    size_t const bytesPerPixel = (pixelOrder == PixelOrder::RGBA ? 4 : 3) * spec::RGB<BitsPerComponent>::bytesPerComponent;
    size_t const stride = bytesPerPixel * width;
    dstBuff.resize(stride * height);
    return Image(pixelOrder, width, height, bytesPerPixel, stride, std::move(dstBuff));
  }
  [[ nodiscard ]] PixelOrder pixelOrder() const {
    return this->pixelOrder_;
  }
  [[ nodiscard ]] uint32_t width() const {
    return this->width_;
  }
  [[ nodiscard ]] uint32_t height() const {
    return this->height_;
  }
  [[ nodiscard ]] uint32_t stride() const {
    return this->stride_;
  }
  [[ nodiscard ]] uint32_t bytesPerPixel() const {
    return this->bytesPerPixel_;
  }
  [[ nodiscard ]] uint8_t bitsPerComponent() const {
    return this->BitsPerComponent;
  }
  [[ nodiscard ]] uint8_t numComponents() const {
    switch (this->pixelOrder_) {
      case PixelOrder::RGB:
        return 3;
      case PixelOrder::RGBA:
        return 4;
      default:
        assert("Do not come here." && (this->pixelOrder_ != PixelOrder::RGB || this->pixelOrder_ != PixelOrder::RGBA));
        return 0;
    }
  }
  [[ nodiscard ]] uint8_t const* data() const {
    return this->data_.data();
  }
  [[ nodiscard ]] uint8_t* data() {
    return this->data_.data();
  }
};

}
