//
// Created by psi on 2020/03/30.
//

extern "C" {
#define register
#include <lcms2.h>
}
#include "Image.hpp"

namespace avif::img {

ColorCoefficients ICCProfile::calcColorCoefficients() const {
  auto rgbProf = cmsOpenProfileFromMem(payload_.data(), payload_.size());
  cmsSetPCS(rgbProf, cmsSigXYZData);
  cmsSetDeviceClass(rgbProf, cmsSigInputClass);

  auto xyzProf = cmsCreateXYZProfile();
  cmsSetPCS(xyzProf, cmsSigXYZData);

  std::vector<cmsHPROFILE> profs = {rgbProf};
  auto transform = cmsCreateMultiprofileTransformTHR(nullptr, profs.data(), profs.size(), TYPE_RGB_8, TYPE_XYZ_FLT, 0,
                                                     0);

  std::vector<uint8_t> srcColor = {0xff, 0x00, 0x00};
  std::vector<float> dstColor = {0x00, 00, 00};
  cmsDoTransform(transform, srcColor.data(), dstColor.data(), 1);
  float const Kr = dstColor[0];

  srcColor = {0x00, 0x00, 0xff};
  cmsDoTransform(transform, srcColor.data(), dstColor.data(), 1);
  float const Kb = dstColor[0];

  return ColorCoefficients(Kr, Kb);
}

}