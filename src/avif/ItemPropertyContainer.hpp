//
// Created by psi on 2019/11/25.
//

#pragma once

#include "Box.hpp"
#include "PixelAspectRatioBox.hpp"
#include "ImageSpatialExtentsProperty.hpp"
#include "PixelInformationProperty.hpp"
#include "AV1CodecConfigurationBox.hpp"

namespace avif {

// https://github.com/nokiatech/heif/blob/master/srcs/common/itempropertycontainer.cpp
// ipco
struct ItemPropertyContainer : public Box {
  using Property =
      std::variant<
          PixelAspectRatioBox,
          ImageSpatialExtentsProperty,
          PixelInformationProperty,
          AV1CodecConfigurationRecordBox>;
  std::vector<Property> properties;
};

}