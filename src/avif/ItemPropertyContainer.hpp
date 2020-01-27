//
// Created by psi on 2019/11/25.
//

#pragma once

#include <variant>

#include "Box.hpp"
#include "PixelAspectRatioBox.hpp"
#include "ImageSpatialExtentsProperty.hpp"
#include "PixelInformationProperty.hpp"
#include "AV1CodecConfigurationBox.hpp"
#include "CleanApertureBox.hpp"
#include "ImageRotationBox.hpp"
#include "ImageMirrorBox.hpp"
#include "ColourInformationBox.hpp"
#include "ContentLightLevelBox.hpp"
#include "MasteringDisplayColourVolumeBox.hpp"
#include "RelativeLocationProperty.hpp"
#include "AuxiliaryTypeProperty.hpp"

namespace avif {

// https://github.com/nokiatech/heif/blob/master/srcs/common/itempropertycontainer.cpp
// ipco
struct ItemPropertyContainer : public Box {
  using Property =
      std::variant<
          PixelAspectRatioBox,
          ImageSpatialExtentsProperty,
          PixelInformationProperty,
          RelativeLocationProperty,
          AuxiliaryTypeProperty,
          CleanApertureBox,
          ImageRotationBox,
          ImageMirrorBox,
          ColourInformationBox,
          ContentLightLevelBox,
          MasteringDisplayColourVolumeBox,
          AV1CodecConfigurationRecordBox>;
  std::vector<Property> properties;
};

}