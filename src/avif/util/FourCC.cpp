//
// Created by psi on 2020/01/12.
//

#include "./FourCC.hpp"

namespace avif::util {

std::string uint2str(uint32_t const code) {
  return std::string{{
                         static_cast<char>((code >> 24u) & 0xffu),
                         static_cast<char>((code >> 16u) & 0xffu),
                         static_cast<char>((code >> 8u) & 0xffu),
                         static_cast<char>((code >> 0u) & 0xffu),
                     }};
}

}