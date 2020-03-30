//
// Created by psi on 2020/01/05.
//

#pragma once

#include <vector>
#include <cstdint>
#include <cassert>
#include <variant>

#include "Spec.hpp"
#include "ColorSpace.hpp"
#include "../ColourInformationBox.hpp"

namespace avif::img {

enum class PixelOrder {
  Mono, /* [Mono], [Mono], ... */
  MonoA, /* [Mono, A], [Mono, A], ... */
  RGB, /* [R,G,B], [R,G,B], ... */
  RGBA, /* [R,G,B,A], [R,G,B,A], ... */
};

class ICCProfile final {
public:
  ICCProfile() = delete;
  ICCProfile(ICCProfile const&) = default;
  ICCProfile(ICCProfile&&) = default;
  ICCProfile& operator=(ICCProfile const&) = default;
  ICCProfile& operator=(ICCProfile&&) = default;

public:
  explicit ICCProfile(std::vector<uint8_t> payload)
  :payload_(std::move(payload)){
  }
  [[ nodiscard ]] std::vector<uint8_t> const& payload() const noexcept {
    return this->payload_;
  }

  [[ nodiscard ]] PrimariesConverter calcColorCoefficients() const;

private:
  std::vector<uint8_t> payload_;
};

using ColorProfile = std::variant<std::monostate, ICCProfile, ColourInformationBox::NCLX>;

template <size_t BitsPerComponent>
class Image final {
  static_assert(BitsPerComponent == 8 || BitsPerComponent == 16);
private:
  static constexpr size_t calcNumComponents(PixelOrder const pixelOrder) noexcept {
    switch (pixelOrder) {
      case PixelOrder::RGB:
        return 3;
      case PixelOrder::RGBA:
        return 4;
      case PixelOrder::Mono:
        return 1;
      case PixelOrder::MonoA:
        return 2;
      default:
        assert(false && "[BUG] Unknown PixelOrder constant!");
    }
  }
private:
  ColorProfile colorProfile_;
  PixelOrder pixelOrder_{};
  uint32_t width_{};
  uint32_t height_{};
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
  explicit Image(ColorProfile colorProfile, PixelOrder pixelOrder, uint32_t width, uint32_t height, uint32_t bytesPerPiexl, uint32_t stride, std::vector<uint8_t> data)
  :colorProfile_(std::move(colorProfile))
  ,pixelOrder_(pixelOrder)
  ,width_(width)
  ,height_(height)
  ,stride_(stride)
  ,data_(std::move(data))
  {
  }
  static Image createEmptyImage(PixelOrder const pixelOrder, uint32_t const width, uint32_t const height) {
    std::vector<uint8_t> dstBuff;
    size_t const bytesPerPixel = calcNumComponents(pixelOrder) * spec::RGB<BitsPerComponent>::bytesPerComponent;
    size_t const stride = bytesPerPixel * width;
    dstBuff.resize(stride * height);
    return Image(avif::img::ColorProfile(), pixelOrder, width, height, bytesPerPixel, stride, std::move(dstBuff));
  }
  [[ nodiscard ]] ColorProfile const& colorProfile() const {
    return this->colorProfile_;
  }
  [[ nodiscard ]] ColorProfile& colorProfile() {
    return this->colorProfile_;
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
    return numComponents() * this->bytesPerComponent();
  }
  [[ nodiscard ]] constexpr uint32_t bytesPerComponent() const {
    return BitsPerComponent / 8;
  }
  [[ nodiscard ]] constexpr uint8_t bitsPerComponent() const {
    return BitsPerComponent;
  }
  [[ nodiscard ]] uint8_t numComponents() const {
    return calcNumComponents(this->pixelOrder_);
  }
  [[ nodiscard ]] uint8_t const* data() const {
    return this->data_.data();
  }
  [[ nodiscard ]] uint8_t* data() {
    return this->data_.data();
  }
  [[ nodiscard ]] bool isMonochrome() {
    switch(pixelOrder_){
      case PixelOrder::Mono:
      case PixelOrder::MonoA:
        return true;
      default:
        return false;
    }
  }
};

}
