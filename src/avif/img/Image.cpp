//
// Created by psi on 2020/03/30.
//

extern "C" {
#define register
#include <lcms2.h>
}
#include "Image.hpp"

namespace avif::img {

PrimariesConverter ICCProfile::calcColorCoefficients() const {
  auto rgbProf = cmsOpenProfileFromMem(payload_.data(), payload_.size());
  cmsSetPCS(rgbProf, cmsSigXYZData);
  cmsSetDeviceClass(rgbProf, cmsSigInputClass);

  std::vector<cmsHPROFILE> profs = {rgbProf};
  auto transform = cmsCreateMultiprofileTransformTHR(nullptr, profs.data(), profs.size(), TYPE_RGB_FLT, TYPE_XYZ_FLT, 0,
                                                     0);

  std::vector<float> srcColor = {1.0f, 0.0f, 0.0f};
  std::vector<float> dstColor = {0.0f, 0.0f, 0.0f};
  cmsDoTransform(transform, srcColor.data(), dstColor.data(), 1);
  float const Kr = dstColor[1];

  srcColor = {0.0f, 0.0f, 1.0f};
  cmsDoTransform(transform, srcColor.data(), dstColor.data(), 1);
  float const Kb = dstColor[1];

  srcColor = {0.0f, 1.0f, 0.0f};
  cmsDoTransform(transform, srcColor.data(), dstColor.data(), 1);
  float const Kg = dstColor[1];

  float const total = Kr + Kg + Kb;

  cmsDeleteTransform(transform);
  cmsCloseProfile(rgbProf);

  return PrimariesConverter(Kr / total, Kb / total);
}

}