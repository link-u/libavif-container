//
// Created by psi on 2019/11/24.
//

#pragma once
#include <utility>
#include <vector>
#include <cstdint>
#include <memory>
#include <variant>
#include <string>
#include <fmt/format.h>

#include "util/Logger.hpp"
#include "util/StreamReader.hpp"
#include "Box.hpp"
#include "FullBox.hpp"
#include "FileBox.hpp"
#include "FileTypeBox.hpp"
#include "PrimaryItemBox.hpp"
#include "ImageMirrorBox.hpp"
#include "ColourInformationBox.hpp"
#include "ContentLightLevelBox.hpp"
#include "MasteringDisplayColourVolumeBox.hpp"
#include "ItemReferenceBox.hpp"

namespace avif {

class Parser final {
public:
  class Error final : std::exception {
  private:
    std::string msg_;
  public:
    template <typename ...Args>
    explicit Error(std::string const& fmt, Args &&... args)
    :std::exception()
    ,msg_(fmt::format(fmt, std::forward<Args>(args)...)){
    }
    explicit Error(std::exception const& err):std::exception(), msg_(fmt::format("[stdlib] {}", err.what()))
    {
    }
    explicit Error(std::string  msg):std::exception(), msg_(std::move(msg)) {
    }
    Error(Error&&) = default;
    Error& operator=(Error&&) = default;
    Error(Error const&) = default;
    Error& operator=(Error const&) = default;
    Error() = delete;
    ~Error() noexcept override = default;
    [[ nodiscard ]] const char* what() const noexcept override {
      return this->msg_.c_str();
    }
    [[ nodiscard ]] std::string const& msg() const noexcept {
      return this->msg_;
    }
  };
  class Result final {
    private:
      std::vector<uint8_t> const buffer_;
      std::variant<FileBox, Parser::Error> const result_;
    public:
      Result(std::vector<uint8_t> buffer, FileBox&& fileBox)
      :buffer_(std::move(buffer))
      ,result_(std::move(fileBox))
      {
      }
      Result(std::vector<uint8_t> buffer, Parser::Error&& err)
          :buffer_(std::move(buffer))
          ,result_(std::move(err))
      {
      }
      ~Result() noexcept = default;
      Result() = delete;
      Result& operator=(Result const&) = delete;
      Result& operator=(Result&&) = delete;
      Result(Result const&) = delete;
      Result(Result&&) = delete;
    public:
      [[ nodiscard ]] bool ok() const { return std::holds_alternative<FileBox>(this->result_); }
      [[ nodiscard ]] std::vector<uint8_t> const& buffer() const { return this->buffer_; }
      [[ nodiscard ]] std::string error() const {
        if (this->ok()) {
          return "<no-error>";
        } else {
          return std::get<Parser::Error>(this->result_).msg();
        }
      }
      [[ nodiscard ]] FileBox const& fileBox() const {
        if(this->ok()) {
          return std::get<FileBox>(this->result_);
        } else {
          throw std::domain_error(fmt::format("ParseResult is an error: {}", error()));
        }
      }
    };

private:
  util::Logger& log_;
private: // intermediate states
  std::vector<uint8_t> buffer_;
  util::StreamReader reader_;
private: // parsed results
  FileBox fileBox_;

private: // internal operations
  Box::Header readBoxHeader();

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
  [[nodiscard]] size_t pos() { return this->reader_.pos(); }
  void seek(size_t pos) { this->reader_.seek(pos); }
  [[nodiscard]] bool consumed() { return this->reader_.consumed(); }
  [[nodiscard]] uint8_t  readU8() { return this->reader_.readU8(); }
  [[nodiscard]] uint16_t readU16() { return this->reader_.readU16(); }
  [[nodiscard]] uint32_t readU32() { return this->reader_.readU32(); }
  [[nodiscard]] uint64_t readU64() { return this->reader_.readU64(); }
  [[nodiscard]] std::optional<uint64_t> readUint(size_t const octets) { return this->reader_.readUint(octets); }
  [[nodiscard]] std::string readString() { return this->reader_.readString(); }

private:
  void parseFullBoxHeader(FullBox& fullBox);
  void warningUnknownBox(Box::Header const& hdr);

private:
  void parseFile();
  void parseBoxInFile();
  void parseFileTypeBox(FileTypeBox& box, size_t end);

  void parseMetaBox(MetaBox& box, size_t end);
  void parseBoxInMeta(MetaBox& box, size_t endOfBox);

  void parseHandlerBox(HandlerBox& box, size_t end);

  void parseItemPropertiesBox(ItemPropertiesBox& box, size_t end);

  void parseBoxInItemPropertyContainer(ItemPropertyContainer &container);

  void parsePixelAspectRatioBox(PixelAspectRatioBox& box, size_t end);
  void parseImageSpatialExtentsProperty(ImageSpatialExtentsProperty &prop, size_t end);
  void parsePixelInformationProperty(PixelInformationProperty& prop, size_t end);
  void parseRelativeLocationProperty(RelativeLocationProperty& rloc, size_t end);
  void parseAuxiliaryTypeProperty(AuxiliaryTypeProperty& aux, size_t end);
  void parseCleanApertureBox(CleanApertureBox& box, size_t end);
  void parseImageRotationBox(ImageRotationBox &box, size_t end);
  void parseImageMirrorBox(ImageMirrorBox& box, size_t end);
  void parseColourInformationBox(ColourInformationBox& box, uint32_t end);
  void parseContentLightLevelBox(ContentLightLevelBox& box, uint32_t end);
  void parseMasteringDisplayColourVolumeBox(MasteringDisplayColourVolumeBox& box, uint32_t end);
  void parseAV1CodecConfigurationRecordBox(AV1CodecConfigurationRecordBox& box, size_t end);

  void parseItemInfoBox(ItemInfoBox& box, size_t end);
  void parseItemInfoEntry(ItemInfoEntry& box, size_t end);

  void parseItemLocationBox(ItemLocationBox& box, size_t end);

  void parsePrimaryItemBox(PrimaryItemBox& box, size_t end);

  void parseItemReferenceBox(ItemReferenceBox& box, size_t end);

  void parseMediaDataBox(MediaDataBox& box, size_t end);

  void parseItemPropertyAssociation(ItemPropertyAssociation &assoc);
};

}
