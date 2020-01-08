//
// Created by psi on 2020/01/05.
//

#pragma once

#include <cstdint>
#include <vector>
#include <optional>
#include <string>
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
  ,buffer_(buffer)
  ,pos_(0)
  {
  }
  ~StreamReader() noexcept = default;

public:
  [[nodiscard]] util::Logger& log() { return this->log_; }
  [[nodiscard]] size_t pos() const { return this->pos_; }
  void seek(size_t pos) {
    this->pos_ = pos;
  }

public:
  [[nodiscard]] uint8_t  readU8();
  [[nodiscard]] uint16_t readU16();
  [[nodiscard]] uint32_t readU32();
  [[nodiscard]] uint64_t readU64();
  [[nodiscard]] std::optional<uint64_t> readUint(size_t octets);
  [[nodiscard]] std::string readString();
  [[nodiscard]] bool consumed() const { return this->pos_ >= this->buffer_.size(); };
};

}



