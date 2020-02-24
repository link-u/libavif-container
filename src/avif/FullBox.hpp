//
// Created by psi on 2019/11/24.
//

#pragma once

#include "Box.hpp"

namespace avif{

struct FullBox : public Box {
public:
  FullBox() = default;
  FullBox(FullBox&&) = default;
  FullBox(FullBox const&) = default;
  FullBox& operator=(FullBox&&) = default;
  FullBox& operator=(FullBox const&) = default;
private:
  uint8_t version_{};
  uint32_t flags_{};
public:
  [[nodiscard]] inline uint8_t version() const { return this->version_; }
  [[nodiscard]] inline uint32_t flags() const { return this->flags_; }

public:
  void setFullBoxHeader(uint8_t version, uint32_t flags) {
    this->version_ = version;
    this->flags_ = flags;
  }
};

}
