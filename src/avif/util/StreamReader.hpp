//
// Created by psi on 2020/01/05.
//

#pragma once

#include <cstdint>
#include <vector>
#include "Logger.hpp"

namespace avif::util {

class StreamReader {
private:
  Logger& log_;
  std::vector<uint8_t> const& buffer_;
  size_t pos_;
public:
  StreamReader() = delete;
  StreamReader(StreamReader&) = delete;
  StreamReader(StreamReader&&) = delete;
  StreamReader& operator=(StreamReader&) = delete;
  StreamReader& operator=(StreamReader&&) = delete;
  explicit StreamReader(util::Logger& log, std::vector<uint8_t> const& buffer)
  :log_(log)
  ,buffer_(buffer){}
  ~StreamReader() noexcept = default;

public:
  [[nodiscard]] util::Logger& log() { return this->log_; }
  [[nodiscard]] size_t pos() { return this->pos_; }
  void seek(size_t pos) {
    this->pos_ = pos;
  }
};

}



