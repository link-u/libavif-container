//
// Created by psi on 2020/01/08.
//

#pragma once

#include "../util/StreamReader.hpp"

namespace avif::av1 {

class BitStreamReader final {
private:
  avif::util::StreamReader reader_;
  avif::util::Logger& log_;
private:
  uint8_t bits_;
  uint8_t posInBits_; // Ordinarily, in [0, 7]. 8 means "End of Buffer".
public:
  BitStreamReader() = delete;
  BitStreamReader(BitStreamReader&&) = delete;
  BitStreamReader(BitStreamReader const&) = delete;
  BitStreamReader& operator=(BitStreamReader&&) = delete;
  BitStreamReader& operator=(BitStreamReader&) = delete;
  explicit BitStreamReader(avif::util::Logger& log, std::vector<uint8_t> const& buffer);
  ~BitStreamReader() noexcept = default;

public:
  [[nodiscard]] size_t posInBits() const;
  [[nodiscard]] size_t posInBytes() const;
  void seekInBytes(size_t posInBytes);
  [[nodiscard]] bool consumed() const;
  [[nodiscard]] uint8_t  readBits(uint8_t bits);
  [[nodiscard]] uint64_t readUint(size_t bits);
  [[nodiscard]] bool  readBool();
  [[nodiscard]] uint8_t  readU8();
  [[nodiscard]] uint16_t readU16();
  [[nodiscard]] uint32_t readU32();
  [[nodiscard]] uint64_t readU64();
  [[nodiscard]] uint32_t readLEB128();
  [[nodiscard]] uint32_t readUVLC();
};

}
