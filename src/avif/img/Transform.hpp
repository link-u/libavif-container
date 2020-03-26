//
// Created by psi on 2020/01/25.
//

#pragma once

#include <cassert>
#include "Image.hpp"
#include "../ImageMirrorBox.hpp"
#include "../ImageRotationBox.hpp"
#include "TransformImpl.hpp"

namespace avif::img {

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
  return dst;
}

template <size_t BitsPerComponent>
Image <BitsPerComponent> rotate(Image<BitsPerComponent> const & src, ImageRotationBox::Rotation const rotation) {
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
  return dst;
}

}