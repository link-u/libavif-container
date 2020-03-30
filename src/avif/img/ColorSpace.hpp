//
// Created by psi on 2020/03/30.
//

#pragma once

#include <tuple>

namespace avif::img {

class PrimariesConverter final {
public:
  PrimariesConverter() = delete;
  constexpr PrimariesConverter(float const Kr, float const Kb)
  :Kr(Kr)
  ,Kb(Kb)
  ,Kg(1.0f - Kr -Kb)
  ,Cb_B(2.0f * (1.0f - Kb))
  ,Cb_G(-2.0f * (1.0f - Kb) * Kb / Kg)
  ,Cr_R(2.0f * (1.0f - Kr))
  ,Cr_G(-2.0f * (1.0f - Kr) * Kr / Kg)
  {

  }
  const float Kr;
  const float Kb;
  const float Kg;

  constexpr void calcYUV(float r, float g, float b, float* y, float* u, float* v) const {
    *y = Kr * r + Kg * g + Kb * b;
    if (u) {
      *u = 0.5f * (b - *y) / (1.0f - Kb);
    }
    if (v) {
      *v = 0.5f * (r - *y) / (1.0f - Kr);
    }
  }

  [[nodiscard]] constexpr std::tuple<float, float, float> calcRGB(float y, float u, float v) const {
    float const r = y             + Cr_R * v;
    float const g = y + Cb_G * u  + Cr_G * v;
    float const b = y + Cb_B * u;
    return std::make_tuple(r, g, b);
  }

private:
  const float Cb_B;
  const float Cb_G;

  const float Cr_R;
  const float Cr_G;

};

}