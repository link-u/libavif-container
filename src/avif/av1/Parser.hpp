//
// Created by psi on 2020/01/05.
//

#pragma once

#include <cstdint>
#include <vector>
#include <variant>
#include <memory>
#include "../util/StreamReader.hpp"
#include "SequenceHeader.hpp"
#include "TemporalDelimiter.hpp"
#include "Padding.hpp"

namespace avif::av1 {


class Parser {
public:
  class Result {
  public:
    class Packet {
    public:
      using Content = std::variant<std::monostate, SequenceHeader, TemporalDelimiter, Padding>;
    private:
      size_t beg_;
      size_t end_;
      size_t size_;
      Header header_;
      Content content_;
    private:
      friend class Parser;
      Packet(size_t const beg, size_t const end, Header const header, Content&& content)
      :beg_(beg)
      ,end_(end)
      ,size_(end-beg)
      ,header_(header)
      ,content_(std::move(content))
      {}
    public:
      [[ nodiscard ]] size_t beg() const { return this->beg_; }
      [[ nodiscard ]] size_t end() const { return this->end_; }
      [[ nodiscard ]] size_t size() const { return this->size_; }
      [[ nodiscard ]] Header const& header() const { return this->header_; }
      [[ nodiscard ]] Header::Type type() const { return this->header_.type; }
      [[ nodiscard ]] Content const& content() const { return this->content_; }
    };

  private:
    std::vector<uint8_t> buffer_;
    std::vector<Packet> packets_;
  public:
    Result(std::vector<uint8_t> buffer, std::vector<Packet> packets)
    :buffer_(std::move(buffer))
    ,packets_(std::move(packets))
    {
    }
    ~Result() noexcept = default;
    Result& operator=(Result const&) = delete;
    Result& operator=(Result&&) = delete;
    Result(Result const&) = delete;
    Result(Result&&) = delete;
  public:
    [[ nodiscard ]] std::vector<uint8_t> const& buffer() const { return this->buffer_; }
    [[ nodiscard ]] std::vector<Packet> const& packets() const { return this->packets_; }
  };
private:
  avif::util::Logger& log_;
private: /* intermediate state */
  std::vector<uint8_t> buffer_;
  avif::util::StreamReader reader_;
  uint8_t bits_;
  uint8_t bitsLeft_;
private:
  std::shared_ptr<Result> result_{};
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
  std::shared_ptr<Result> parse();

private:
  std::optional<Result::Packet> parsePacket();

  // General OBU
  Header parseHeader();

  // Sequence Header OBU
  SequenceHeader parseSequenceHeader();
  SequenceHeader::TimingInfo parseTimingInfo();
  SequenceHeader::DecoderModelInfo parseDecoderModelInfo();
  SequenceHeader::ColorConfig parseColorConfig(SequenceHeader const& shdr);

private:
  [[nodiscard]] size_t posInBits() {
    return (this->reader_.pos() - 1) * 8 + (8 - bitsLeft_);
  }
  [[nodiscard]] size_t posInBytes() {
    return bitsLeft_ == 0 ? reader_.pos() : (reader_.pos() - 1);
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



