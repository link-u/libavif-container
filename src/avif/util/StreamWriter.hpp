//
// Created by psi on 2019/11/25.
//

#pragma once


#include <cstdint>
#include <vector>

namespace avif::util {

class StreamWriter {
private:
  std::vector<uint8_t> buff_;
public:
  StreamWriter() = default;
  StreamWriter(StreamWriter &&) = delete;
  StreamWriter(StreamWriter const&) = delete;
  StreamWriter& operator=(StreamWriter &&) = delete;
  StreamWriter& operator=(StreamWriter const&) = delete;

public:
  [[nodiscard]] std::vector<uint8_t> const& buffer() const { return this->buff_; };
  [[nodiscard]] size_t size() const { return this->buff_.size(); };
  void putU8(uint8_t data);
  void putU16L(uint16_t data);
  void putU16B(uint16_t data);
  void putU32L(uint32_t data);
  void putU32B(uint32_t data);
  void putU64L(uint64_t data);
  void putU64B(uint64_t data);
  void append(std::vector<uint8_t> const& data);
  void append(uint8_t const* data, size_t length);
};

}