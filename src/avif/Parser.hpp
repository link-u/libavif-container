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
  struct Result {
    std::vector<uint8_t> const buffer;

  };
private:
  util::Logger& log_;
  size_t pos_;
  std::vector<uint8_t> const buffer_;

private: // internal operations
  uint8_t  readU8();
  uint16_t readU16();
  uint32_t readU32();
  uint64_t readU64();
  std::optional<uint64_t> readUint(size_t octets);

private: // parsed results
  std::shared_ptr<FileBox> fileBox_;

public:
  Parser() = delete;
  Parser(Parser&&) = delete;
  Parser(Parser const&) = delete;
  Parser operator=(Parser&&) = delete;
  Parser operator=(Parser const&) = delete;

public: //entry point
  Parser(util::Logger& log, std::vector<uint8_t> buff);
  std::variant<std::shared_ptr<FileBox>, std::string> parse();

public: // getters
  [[nodiscard]] util::Logger& log() { return this->log_; }
  [[nodiscard]] std::vector<uint8_t> const& buffer() { return this->buffer_; }

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
