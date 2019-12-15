//
// Created by psi on 2019/11/25.
//

#pragma once

#include "Box.hpp"
#include "PixelAspectRatioBox.hpp"
#include "ImageSpatialExtentsProperty.hpp"
#include "PixelInformationProperty.hpp"

namespace avif {

// https://github.com/nokiatech/heif/blob/master/srcs/common/itempropertycontainer.cpp
// ipco
struct ItemPropertyContainer : public Box {
  // tupleの左側のidは、ItemPropertyAssociationに対応している。
  // https://github.com/nokiatech/heif/blob/2666d0745b7e1bcb0520b60686d50c8c4f9be689/srcs/common/itempropertiesbox.hpp#L59
  std::tuple<uint8_t, PixelAspectRatioBox> pixelAspectRatioBox;
  std::tuple<uint8_t, ImageSpatialExtentsProperty> imageSpatialExtentsProperty;
  std::tuple<uint8_t, PixelInformationProperty> pixelInformationProperty;
};

}