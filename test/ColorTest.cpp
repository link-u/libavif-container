//
// Created by psi on 2020/01/11.
//

#include <vector>
#include <memory>
#include <gtest/gtest.h>
#include "../src/avif/img/Conversion.hpp"

TEST(ColorTest, LimitedTest) {
  using namespace avif::img;
  auto converter = converters::MC_BT_2020_NCL;
  { // Red color
    uint16_t y = 0;
    uint16_t u = 0;
    uint16_t v = 0;
    detail::calcYUV<decltype(converter), 8, 12, false, false>(converter, 255, 0, 0, &y, &u, &v);
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    std::tie(r,g,b) = detail::calcRGB<decltype(converter), 8, 12, false, false>(converter, &y, &u, &v);
    ASSERT_EQ(255, r);
    ASSERT_EQ(0, g);
    ASSERT_EQ(0, b);
  }
  { // White must have max luma and no chroma.
    uint8_t y = 0;
    uint8_t u = 0;
    uint8_t v = 0;
    detail::calcYUV<decltype(converter), 8, 8, false, false>(converter, 255, 255, 255, &y, &u, &v);
    ASSERT_EQ(235, y);
    ASSERT_EQ(128, u);
    ASSERT_EQ(128, v);
  }
}

TEST(ColorTest, FullTest) {
  using namespace avif::img;
  auto converter = converters::MC_BT_2020_NCL;
  { // Red color
    uint16_t y = 0;
    uint16_t u = 0;
    uint16_t v = 0;
    detail::calcYUV<decltype(converter), 8, 12, false, true>(converter, 255, 0, 0, &y, &u, &v);
    auto [r,g,b] = detail::calcRGB<decltype(converter), 8, 12, false, true>(converter, &y, &u, &v);
    ASSERT_EQ(255, r);
    ASSERT_EQ(0, g);
    ASSERT_EQ(0, b);
  }
  { // White must have max luma and no chroma.
    uint8_t y = 0;
    uint8_t u = 0;
    uint8_t v = 0;
    detail::calcYUV<decltype(converter), 8, 8, false, true>(converter, 255, 255, 255, &y, &u, &v);
    ASSERT_EQ(255, y);
    ASSERT_EQ(128, u);
    ASSERT_EQ(128, v);
  }
}