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

std::string uint2str(uint32_t code);

}