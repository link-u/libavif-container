//
// Created by psi on 2020/01/11.
//

#include <vector>
#include <memory>
#include <gtest/gtest.h>
#include "../src/avif/img/Conversion.hpp"

TEST(ColorTest, RedTest) {
  using namespace avif::img;
  uint8_t y = 0;
  uint8_t u = 0;
  uint8_t v = 0;
  detail::calcYUV<8,8>(255, 0, 0, &y, &u, &v);
  uint8_t r = 0;
  uint8_t g = 0;
  uint8_t b = 0;
  std::tie(r,g,b) = detail::calcRGB<8,8>(&y, &u, &v);
  ASSERT_TRUE(254 <= r);
  ASSERT_TRUE(g <= 1);
  ASSERT_TRUE(b <= 1);
}