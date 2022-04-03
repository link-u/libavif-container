//
// Created by psi on 2022/03/29.
//

#include <vector>
#include <memory>
#include <gtest/gtest.h>
#include "../../src/avif/math/Fraction.hpp"

TEST(FractionTest, Basic) {
  using avif::math::Fraction;
  auto const f = Fraction(1,1);
  ASSERT_TRUE(f.isInteger());
  auto const h = f.div(2);
  ASSERT_FALSE(h.isInteger());
  ASSERT_EQ(1, h.numerator());
  ASSERT_EQ(2, h.denominator());
  auto const g = f.div(-2);
  ASSERT_FALSE(g.isInteger());
  ASSERT_EQ(-1, g.numerator());
  ASSERT_EQ(+2, g.denominator());
}

TEST(FractionTest, Calc) {
  using avif::math::Fraction;
  auto const f = Fraction(1,1);
  auto const h = f.div(2);
  auto const g = f.minus(h);
  ASSERT_EQ(1, g.numerator());
  ASSERT_EQ(+2, g.denominator());
  auto const z = g.minus(h);
  ASSERT_EQ(0, z.numerator());
  ASSERT_EQ(1,z.denominator());
}

TEST(FractionTest, Minus) {
  using avif::math::Fraction;
  auto const f = Fraction(1,-1);
  ASSERT_EQ(-1, f.numerator());
  ASSERT_EQ(1, f.denominator());
  auto const h = f.div(-2);
  ASSERT_EQ(1, h.numerator());
  ASSERT_EQ(2, h.denominator());

  auto const one = Fraction(1, 1);
  auto const half = one.div(2);
  auto const v = half.minus(one);
  ASSERT_EQ(-1, v.numerator());
  ASSERT_EQ(2, v.denominator());
}
