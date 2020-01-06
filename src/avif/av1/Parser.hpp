//
// Created by psi on 2020/01/05.
//

#pragma once

#include <cstdint>
#include <vector>
#include <variant>
#include "../util/StreamReader.hpp"
#include "SequenceHeader.hpp"
#include "TemporalDelimiter.hpp"
#include "Padding.hpp"

namespace avif::av1 {

class Parser {
private:
  avif::util::Logger& log_;
  std::vector<uint8_t> buffer_;
  avif::util::StreamReader reader_;
  uint8_t bits_;
  uint8_t bitsLeft_;
private:
  uint16_t OperatingPointIdc = 0;
public:
  Parser() = delete;
  Parser(Parser&&) = delete;
  Parser(Parser const&) = delete;
  Parser operator=(Parser&&) = delete;
  Parser operator=(Parser const&) = delete;

public: //entry point
  Parser(util::Logger& log, std::vector<uint8_t> buffer);
  std::variant<std::monostate, SequenceHeader, TemporalDelimiter, Padding> parse();

private:
  Header parseHeader();
  SequenceHeader parseSequenceHeader(Header const& hdr);
  SequenceHeader::TimingInfo parseTimingInfo();
  SequenceHeader::DecoderModelInfo parseDecoderModelInfo();
  SequenceHeader::ColorConfig parseColorConfig(SequenceHeader const& shdr);

private:
  [[nodiscard]] size_t posInBits() {
    return (this->reader_.pos() - 1) * 8 + (8 - bitsLeft_);
  }
  [[nodiscard]] bool consumed() { return this->reader_.consumed(); }
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



