//
// Created by psi on 2019/11/24.
//

#pragma once
#include <vector>
#include <cstdint>
#include <memory>
#include <variant>
#include <string>
#include "util/Logger.hpp"
#include "Box.hpp"
#include "FullBox.hpp"
#include "FileBox.hpp"
#include "FileTypeBox.hpp"

namespace avif {

class Parser final {
public:
  class Result final {
    friend class Parser;
    private:
      std::vector<uint8_t> const buffer_;
      std::variant<std::shared_ptr<const FileBox>, std::string> const result_;
    public:
      Result(std::vector<uint8_t> buffer, std::variant<std::shared_ptr<const FileBox>, std::string> result)
      :buffer_(std::move(buffer))
      ,result_(std::move(result))
      {
      }
      ~Result() noexcept = default;
      Result& operator=(Result const&) = delete;
      Result& operator=(Result&&) = delete;
      Result(Result const&) = delete;
      Result(Result&&) = delete;
    public:
      [[ nodiscard ]] bool isSuccess() const { return std::holds_alternative<std::shared_ptr<const FileBox>>(this->result_); }
      [[ nodiscard ]] std::vector<uint8_t> const& buffer() const { return this->buffer_; }
      [[ nodiscard ]] std::string error() const {
        if (this->isSuccess()) {
          return std::get<1>(this->result_);
        } else {
          return "";
        }
      }
      [[ nodiscard ]] std::shared_ptr<const FileBox> fileBox() const {
        if(this->isSuccess()) {
          return std::get<0>(this->result_);
        } else {
          return std::shared_ptr<const FileBox>();
        }
      }
    };
private:
  util::Logger& log_;
private: // intermediate states
  size_t pos_;
  std::vector<uint8_t> buffer_;
private: // parsed results
  std::shared_ptr<FileBox> fileBox_;

private: // internal operations
  uint8_t  readU8();
  uint16_t readU16();
  uint32_t readU32();
  uint64_t readU64();
  std::optional<uint64_t> readUint(size_t octets);

private:
  std::shared_ptr<Result> result_;

public:
  Parser() = delete;
  Parser(Parser&&) = delete;
  Parser(Parser const&) = delete;
  Parser operator=(Parser&&) = delete;
  Parser operator=(Parser const&) = delete;

public: //entry point
  Parser(util::Logger& log, std::vector<uint8_t> buff);
  std::shared_ptr<Result> parse();

public: // getters
  [[nodiscard]] util::Logger& log() { return this->log_; }

private:
  void parseFullBoxHeader(FullBox& fullBox);

private:
  std::optional<std::string> parseFile();
  std::optional<std::string> parseBoxInFile();
  std::optional<std::string> parseFileTypeBox(FileTypeBox& box, size_t end);

  std::optional<std::string> parseMetaBox(MetaBox& box, size_t end);
  std::optional<std::string> parseBoxInMeta(MetaBox& box, size_t endOfBox);

  std::optional<std::string> parseHandlerBox(HandlerBox& box, size_t end);

  std::optional<std::string> parseItemPropertiesBox(ItemPropertiesBox& box, size_t end);

  std::optional<std::string> parseItemPropertyContainer(ItemPropertyContainer &container);
  std::optional<std::string> parseBoxInItemPropertyContainer(uint8_t id, ItemPropertyContainer &container);

  std::optional<std::string> parsePixelAspectRatioBox(PixelAspectRatioBox& box, size_t end);
  std::optional<std::string> parseImageSpatialExtentsProperty(ImageSpatialExtentsProperty &prop, size_t end);
  std::optional<std::string> parsePixelInformationProperty(PixelInformationProperty& prop, size_t end);

  std::optional<std::string> parseItemLocationBox(ItemLocationBox& box, size_t end);

  std::optional<std::string> parseMediaDataBox(MediaDataBox& box, size_t end);

  std::optional<std::string> parseItemPropertyAssociation(ItemPropertyAssociation &assoc);
};

}
