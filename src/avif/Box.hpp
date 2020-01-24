//
// Created by psi on 2019/11/24.
//

#pragma once

#include <optional>
#include <string>

namespace avif {

struct Box {
  struct Header {
    uint32_t offset;
    uint32_t size;
    uint32_t type;
    [[ nodiscard ]] uint32_t end() const {
      return this->offset + this->size;
    }
  };
public:
  Header hdr;
public:
  Box() = default;
  Box(Box&&) = default;
  Box(Box const&) = default;
  Box& operator=(Box&&) = default;
  Box& operator=(Box const&) = default;
};

}
