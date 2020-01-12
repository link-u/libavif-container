//
// Created by psi on 2020/01/12.
//

#pragma once

#include <cstdint>
#include <string>

namespace avif::util {

constexpr uint32_t str2uint(const char str[4]) {
  return
      static_cast<uint32_t>(str[0]) << 24u |
      static_cast<uint32_t>(str[1]) << 16u |
      static_cast<uint32_t>(str[2]) << 8u |
      static_cast<uint32_t>(str[3]) << 0u;
}

std::string uint2str(uint32_t const code) {
  char str[5];
  str[0] = static_cast<uint8_t>((code >> 24u) & 0xffu);
  str[1] = static_cast<uint8_t>((code >> 16u) & 0xffu);
  str[2] = static_cast<uint8_t>((code >>  8u) & 0xffu);
  str[3] = static_cast<uint8_t>((code >>  0u) & 0xffu);
  str[4] = '\0';
  return std::string(str);
}

}