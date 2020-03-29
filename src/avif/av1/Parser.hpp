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
#include "BitStreamReader.hpp"

namespace avif::av1 {

class Parser {
public:
  class Error final : std::exception {
  private:
    std::string msg_;
  public:
    Error() = delete;
    template <typename ...Args>
    explicit Error(std::string const& fmt, Args &&... args)
    :std::exception()
     ,msg_(fmt::format(fmt.c_str(), std::forward<Args>(args)...))
    {
    }
    explicit Error(std::exception const& err):std::exception(), msg_(fmt::format("[stdlib] {}", err.what())) {
    }
    explicit Error(std::string  msg):std::exception(), msg_(std::move(msg)) {
    }
    Error(Error&&) = default;
    Error& operator=(Error&&) = default;
    Error(Error const&) = default;
    Error& operator=(Error const&) = default;
    ~Error() noexcept override = default;
    [[ nodiscard ]] const char* what() const noexcept override {
      return this->msg_.c_str();
    }
    [[ nodiscard ]] std::string const& msg() const noexcept {
      return this->msg_;
    }
  };
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
    std::variant<std::vector<Packet>, Error> const result_;
  public:
    Result() = delete;
    explicit Result(std::vector<uint8_t> buffer, std::vector<Packet> packets)
    :buffer_(std::move(buffer))
    ,result_(std::move(packets))
    {
    }
    explicit Result(std::vector<uint8_t> buffer, Error error)
    :buffer_(std::move(buffer))
    ,result_(std::move(error))
    {
    }
    ~Result() noexcept = default;
    Result& operator=(Result const&) = delete;
    Result& operator=(Result&&) = delete;
    Result(Result const&) = delete;
    Result(Result&&) = delete;
  public:
    [[ nodiscard ]] bool ok() const { return std::holds_alternative<std::vector<Packet>>(this->result_); }
    [[ nodiscard ]] std::vector<uint8_t> const& buffer() const { return this->buffer_; }
    [[ nodiscard ]] std::string error() const {
      if (this->ok()) {
        return "<no-error>";
      } else {
        return std::get<Parser::Error>(this->result_).msg();
      }
    }
    [[ nodiscard ]] std::vector<Packet> const& packets() const {
      if(this->ok()) {
        return std::get<std::vector<Packet>>(this->result_);
      } else {
        throw std::domain_error(fmt::format("ParseResult is an error: {}", error()));
      }
    }
  };
private:
  avif::util::Logger& log_;
private: /* intermediate states */
  std::vector<uint8_t> buffer_;
  BitStreamReader reader_;
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
  [[nodiscard]] size_t posInBits() { return this->reader_.posInBits(); }
  [[nodiscard]] size_t posInBytes() { return this->reader_.posInBytes(); }
  void seekInBytes(size_t posInBytes) { this->reader_.seekInBytes(posInBytes); };
  [[nodiscard]] bool consumed() { return this->reader_.consumed(); }
  [[nodiscard]] uint8_t  readBits(uint8_t bits) { return this->reader_.readBits(bits); }
  [[nodiscard]] uint64_t readUint(size_t bits) { return this->reader_.readUint(bits); };
  [[nodiscard]] bool  readBool() { return this->reader_.readBool(); };
  [[nodiscard]] uint8_t  readU8() { return this->reader_.readU8(); };
  [[nodiscard]] uint16_t readU16() { return this->reader_.readU16(); };
  [[nodiscard]] uint32_t readU32() { return this->reader_.readU32(); };
  [[nodiscard]] uint64_t readU64() { return this->reader_.readU64(); };
  [[nodiscard]] uint32_t readLEB128() { return this->reader_.readLEB128(); };
  [[nodiscard]] uint32_t readUVLC() { return this->reader_.readUVLC(); };
};

}



