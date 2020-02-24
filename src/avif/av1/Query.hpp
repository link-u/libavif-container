//
// Created by psi on 2020/02/25.
//

#pragma once

#include <optional>
#include <vector>
#include "./Parser.hpp"

namespace avif::av1::util::query {

template <typename T>
std::optional<T> find(std::vector<Parser::Result::Packet> const& packets) {
  for(avif::av1::Parser::Result::Packet const& packet : packets) {
    if(std::holds_alternative<T>(packet.content())) {
      return std::optional<T>(std::get<T>(packet.content()));
    }
  }
  return std::optional<T>();
}

}