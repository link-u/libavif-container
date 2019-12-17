//
// Created by psi on 2019/11/23.
//

#include <cstdio>
#include <string>
#include <memory>

#include "../../external/tinyformat/tinyformat.h"
#include "util/Logger.hpp"

#include "Parser.hpp"
#include "FileBox.hpp"
#include "ItemPropertiesBox.hpp"
#include "ItemPropertyContainer.hpp"

namespace {
constexpr uint32_t str2uint(const char str[4]) {
  return
      static_cast<uint32_t>(str[0]) << 24u |
      static_cast<uint32_t>(str[1]) << 16u |
      static_cast<uint32_t>(str[2]) << 8u |
      static_cast<uint32_t>(str[3]) << 0u;
}
std::string uint2str(uint32_t const code) {
  char str[5];
  str[0] = static_cast<uint8_t>((code >> 24u) & 0xffu);
  str[1] = static_cast<uint8_t>((code >> 16u) & 0xffu);
  str[2] = static_cast<uint8_t>((code >>  8u) & 0xffu);
  str[3] = static_cast<uint8_t>((code >>  0u) & 0xffu);
  str[4] = '\0';
  return std::string(str);
}
}

namespace avif {

uint8_t Parser::readU8() {
  uint8_t res = this->buffer_.at(pos_);
  pos_++;
  return res;
}

uint16_t Parser::readU16() {
  uint16_t res =
      static_cast<uint16_t>(static_cast<uint16_t>(buffer_.at(pos_)) << 8u) |
      static_cast<uint16_t>(static_cast<uint16_t>(this->buffer_.at(pos_ + 1)) << 0u);
  pos_+=2;
  return res;
}

uint32_t Parser::readU32() {
  uint32_t res =
      static_cast<uint32_t>(buffer_.at(pos_ + 0)) << 24u |
      static_cast<uint32_t>(buffer_.at(pos_ + 1)) << 16u |
      static_cast<uint32_t>(buffer_.at(pos_ + 2)) << 8u |
      static_cast<uint32_t>(buffer_.at(pos_ + 3)) << 0u;
  pos_+=4;
  return res;
}

uint64_t Parser::readU64() {
  uint64_t res =
      static_cast<uint64_t>(buffer_.at(pos_ + 0)) << 56u |
      static_cast<uint64_t>(buffer_.at(pos_ + 1)) << 48u |
      static_cast<uint64_t>(buffer_.at(pos_ + 2)) << 40u |
      static_cast<uint64_t>(buffer_.at(pos_ + 3)) << 32u |
      static_cast<uint64_t>(buffer_.at(pos_ + 4)) << 24u |
      static_cast<uint64_t>(buffer_.at(pos_ + 5)) << 16u |
      static_cast<uint64_t>(buffer_.at(pos_ + 6)) << 8u |
      static_cast<uint64_t>(buffer_.at(pos_ + 7)) << 0u;
  pos_+=8;
  return res;
}
std::optional<uint64_t> Parser::readUint(size_t octets) {
  switch(octets){
    case 0:
      return std::make_optional(0);
    case 1:
      return std::make_optional(readU8());
    case 2:
      return std::make_optional(readU16());
    case 4:
      return std::make_optional(readU32());
    case 8:
      return std::make_optional(readU64());
    default:
      return std::optional<uint64_t>();
  }
}

//-----------------------------------------------------------------------------
constexpr uint32_t boxType(const char *str) {
  return str2uint(str);
}
//-----------------------------------------------------------------------------

Parser::Parser(util::Logger& log, std::vector<uint8_t> buff)
: log_(log)
, pos_(0)
, buffer_(std::move(buff))
, fileBox_(std::make_shared<FileBox>())
{
}

std::shared_ptr<Parser::Result> Parser::parse() {
  if(this->result_) {
    return this->result_;
  }
  std::optional<std::string> res = this->parseFile();
  if(res.has_value()){
    this->result_ = std::make_shared<Parser::Result>(std::move(this->buffer_), std::variant<std::shared_ptr<const FileBox>, std::string>(res.value()));
  } else {
    this->result_ = std::make_shared<Parser::Result>(std::move(this->buffer_), std::variant<std::shared_ptr<const FileBox>, std::string>(fileBox_));
  }
  return this->result_;
}

std::optional<std::string> Parser::parseFile() {
  while(this->pos_ < this->buffer_.size()) {
    std::optional<std::string> result(this->parseBoxInFile());
    if(result.has_value()) {
      return std::move(result);
    }
  }
  return std::optional<std::string>();
}

std::optional<std::string> Parser::parseBoxInFile() {
  size_t const beg = this->pos_;
  size_t const size = readU32();
  size_t const type = readU32();
  size_t const end = this->pos_ - 8 + size;
  std::optional<std::string> res{};
  std::shared_ptr<FileBox> box = this->fileBox_;
  switch(type) {
    case boxType("ftyp"):
      // ISO/IEC 14496-12:2015(E)
      // 4.3 File Type Box
      res = this->parseFileTypeBox(box->fileTypeBox, end);
      break;
    case boxType("free"):
    case boxType("skip"):
      // ISO/IEC 14496-12:2015(E)
      // 8.1.2
      // free_type may be ‘free’ or ‘skip’.
      break;
    case boxType("meta"):
      // ISO/IEC 14496-12:2015(E)
      // 8.11.1
      // free_type may be ‘free’ or ‘skip’.
      res = this->parseMetaBox(box->metaBox, end);
      break;
    case boxType("mdat"):
      res = this->parseMediaDataBox(box->mediaDataBox, end);
      break;
    default:
      std::string typeStr = uint2str(type);
      log().warn("Unknown box type=%s(=0x%0lx) with size=%ld(%ld~%ld/%ld)", typeStr.c_str(), type, size, beg, end, this->buffer_.size());
      break;
  }
  if(res.has_value()) {
    this->pos_ = beg;
    return std::move(res);
  }
  this->pos_ = end;
  return std::optional<std::string>();
}

std::optional<std::string> Parser::parseFileTypeBox(FileTypeBox& box, size_t const end) {
  uint32_t const majorBrand = readU32();
  uint32_t const minorVersion = readU32();
  if(str2uint("avif") != majorBrand) {
    return std::make_optional(tfm::format("Unsupported brand: %s", uint2str(majorBrand)));
  }
  if(minorVersion != 0) {
    return std::make_optional(tfm::format("We currently just support version 0(!=%d)", minorVersion));
  }
  std::vector<std::string> compatibleBrands;
  while(this->pos_ < end) {
    uint32_t const compatBrand = readU32();
    compatibleBrands.emplace_back(uint2str(compatBrand));
  }
  box.majorBrand = uint2str(majorBrand);
  box.minorVersion = minorVersion;
  box.compatibleBrands = std::move(compatibleBrands);
  return std::optional<std::string>();
}

// See ISOBMFF p.90
std::optional<std::string> Parser::parseMetaBox(MetaBox& box, size_t const end) {
  // MetaBox extends FullBox.
  // See: ISOBMFF p.21
  this->parseFullBoxHeader(box);
  while(this->pos_ < end) {
    std::optional<std::string> res = this->parseBoxInMeta(box, end);
    if(res.has_value()) {
      return std::move(res);
    }
  }
  return std::optional<std::string>();
}

std::optional<std::string> Parser::parseBoxInMeta(MetaBox& box, size_t const endOfBox) {
  size_t const beg = this->pos_;
  size_t const size = readU32();
  size_t const type = readU32();
  size_t const end = this->pos_ - 8 + size;
  std::optional<std::string> res{};
  switch(type) {
    case boxType("hdlr"):
      res = this->parseHandlerBox(box.handlerBox, end);
      break;
    case boxType("iprp"):
      // 詳しい定義は、HEVCの仕様書買わないとわからん。
      this->parseItemPropertiesBox(box.itemPropertiesBox, end);
      break;
    case boxType("iinf"):
      // 8.11.6 Item Information Box
      // See: ISOBMFF p.95
      // TODO: parse
      break;
    case boxType("iloc"):
      // 8.11.3 The Item Location Box
      // See: ISOBMFF p.91
      this->parseItemLocationBox(box.itemLocationBox, end);
      break;
    default:
      std::string typeStr = uint2str(type);
      log().warn("Unknown box type=%s(=0x%0lx) with size=%ld(%ld~%ld/%ld) in MetaBox", typeStr.c_str(), type, size, beg, end, endOfBox);
      break;
  }
  if(res.has_value()) {
    this->pos_ = beg;
    return std::move(res);
  }
  this->pos_ = end;
  return std::optional<std::string>();
}

std::optional<std::string> Parser::parseHandlerBox(HandlerBox& box, size_t const end) {
  // HandlerBox extends FullBox.
  // See: ISOBMFF p.43
  this->parseFullBoxHeader(box);

  // ISOBMFF p.44 8.4.3.2
  readU32(); // pre_defined = 0
  uint32_t const handlerType = readU32();
  readU32(); // reserved
  readU32(); // reserved
  readU32(); // reserved
  std::string name(std::next(this->buffer_.begin(), this->pos_), std::next(this->buffer_.begin(), end));
  switch(handlerType) {
    case str2uint("pict"):
      // よくわからんが、これで正しいらしい
      // たぶんHEIFの仕様書に書いてある…
      // https://github.com/AOMediaCodec/libavif/blob/c673f2e884e635ff8e9b2c2951a2dddd2a00ffc3/src/write.c#L185
      // https://github.com/Kagami/avif.js/blob/500e3357e5d56750dbd4e40b3aee74b13207721e/mov.js#L49
      box.handler = "pict";
      break;
    case str2uint("null"):
      log().warn("NULL header type in HeaderBox");
      break;
    default:
      log().warn("Unknown header type=%s in HeaderBox", uint2str(handlerType));
      break;
  }
  return std::optional<std::string>();
}

std::optional<std::string> Parser::parseItemPropertiesBox(ItemPropertiesBox& box, size_t const end) {
  // https://github.com/nokiatech/heif/blob/master/srcs/common/itempropertiesbox.cpp
  std::optional<std::string> res = this->parseItemPropertyContainer(box.itemPropertyContainer);
  if(res.has_value()){
    return std::move(res);
  }
  // read "ipma"s
  while(this->pos_ < end) {
    // TODO: コードの整理
    size_t const beg = this->pos_;
    size_t const size = readU32();
    size_t const type = readU32();
    size_t const endIpma = this->pos_ - 8 + size;
    if(type != str2uint("ipma")) {
      return std::make_optional(tinyformat::format("ItemPropertyAssociation expected, got %s", uint2str(type)));
    }
    ItemPropertyAssociation itemPropertyAssociation;
    res = this->parseItemPropertyAssociation(itemPropertyAssociation);
    if(res.has_value()){
      this->pos_ = beg;
      return std::move(res);
    }
    box.itemPropertyAssociations.emplace_back(itemPropertyAssociation);
    this->pos_ = endIpma;
  }
  return std::optional<std::string>();
}

std::optional<std::string> Parser::parseItemPropertyContainer(ItemPropertyContainer& container) {
  // https://github.com/nokiatech/heif/blob/master/srcs/common/itempropertycontainer.cpp
  size_t const beg = this->pos_;
  size_t const size = readU32();
  size_t const type = readU32();
  size_t const end = this->pos_ - 8 + size;
  if(type != str2uint("ipco")) {
    return std::make_optional(tinyformat::format("ItemPropertyContainer expected, got %s", uint2str(type)));
  }
  uint8_t id = 0;
  while(this->pos_ < end) {
    std::optional<std::string> res = this->parseBoxInItemPropertyContainer(id, container);
    if(res.has_value()) {
      this->pos_ = beg;
      return std::move(res);
    }
    id++;
  }
  this->pos_ = end;
  return std::optional<std::string>();
}

std::optional<std::string> Parser::parseBoxInItemPropertyContainer(uint8_t id, ItemPropertyContainer& container) {
  // https://github.com/nokiatech/heif/blob/master/srcs/common/itempropertycontainer.cpp
  size_t const beg = this->pos_;
  size_t const size = readU32();
  size_t const type = readU32();
  size_t const end = this->pos_ - 8 + size;
  std::optional<std::string> res{};
  switch(type) {
    case boxType("pasp"):
      std::get<0>(container.pixelAspectRatioBox) = id;
      res = this->parsePixelAspectRatioBox(std::get<1>(container.pixelAspectRatioBox), end);
      break;
    case boxType("ispe"):
      std::get<0>(container.imageSpatialExtentsProperty) = id;
      res = this->parseImageSpatialExtentsProperty(std::get<1>(container.imageSpatialExtentsProperty), end);
      break;
    case boxType("pixi"):
      std::get<0>(container.pixelInformationProperty) = id;
      res = this->parsePixelInformationProperty(std::get<1>(container.pixelInformationProperty), end);
      break;
    case boxType("av1C"):
      // https://aomediacodec.github.io/av1-isobmff/#av1codecconfigurationbox-section
      // TODO: parse.
      break;
    case boxType("free"):
    case boxType("skip"):
      // ISO/IEC 14496-12:2015(E)
      // 8.1.2
      // free_type may be ‘free’ or ‘skip’.
      break;
    default:
      std::string typeStr = uint2str(type);
      log().warn("Unknown box type=%s(=0x%0lx) with size=%ld(%ld~%ld/%ld) in In ItemPropertyContainer", typeStr.c_str(), type, size, beg, end, this->buffer_.size());
      break;
  }
  if(res.has_value()) {
    this->pos_ = beg;
    return std::move(res);
  }
  this->pos_ = end;
  return std::optional<std::string>();
}

std::optional<std::string> Parser::parsePixelAspectRatioBox(PixelAspectRatioBox& box, size_t end) {
  // 12.1.4 Pixel Aspect Ratio and Clean Aperture
  // See: ISOBMFF p.170
  box.hSpacing = readU32();
  box.vSpacing = readU32();
  return std::optional<std::string>();
}

std::optional<std::string> Parser::parseImageSpatialExtentsProperty(ImageSpatialExtentsProperty& prop, size_t end) {
  // https://github.com/nokiatech/heif/blob/master/srcs/common/pixelinformationproperty.cpp
  parseFullBoxHeader(prop);
  prop.imageWidth = readU32();
  prop.imageHeight = readU32();
  return std::optional<std::string>();
}

std::optional<std::string> Parser::parsePixelInformationProperty(PixelInformationProperty& prop, size_t end) {
  // https://github.com/nokiatech/heif/blob/master/srcs/common/pixelinformationproperty.cpp
  parseFullBoxHeader(prop);
  prop.numChannels = readU8();
  for(uint8_t i =0; i < prop.numChannels; ++i) {
    prop.bitsPerChannel.emplace_back(readU8());
  }
  return std::optional<std::string>();
}

std::optional<std::string> Parser::parseItemPropertyAssociation(ItemPropertyAssociation& assoc) {
  // https://github.com/nokiatech/heif/blob/master/srcs/common/itempropertyassociation.cpp
  parseFullBoxHeader(assoc);
  assoc.itemCount = readU32();
  for(uint32_t i = 0; i < assoc.itemCount; ++i) {
    ItemPropertyAssociation::Item item;
    if(assoc.version() < 1) {
      item.itemID = readU16();
    } else {
      item.itemID = readU32();
    }
    item.entryCount = readU8();
    for(uint8_t j = 0; j < item.entryCount; ++j) {
      ItemPropertyAssociation::Item::Entry entry{};
      if((assoc.flags() & 1u) == 1u) {
        uint16_t const v = readU16();
        entry.essential = (v & 0x8000u) == 0x8000u;
        entry.propertyIndex = v & 0x7fffu;
      } else {
        uint8_t const v = readU8();
        entry.essential = (v & 0x80u) == 0x80u;
        entry.propertyIndex = v & 0x7fu;
      }
      item.entries.emplace_back(entry);
    }
    assoc.items.emplace_back(item);
  }
  return std::optional<std::string>();
}

std::optional<std::string> Parser::parseItemLocationBox(ItemLocationBox& box, size_t const end) {
  // 8.11.3 The Item Location Box
  // See: ISOBMFF p.91
  this->parseFullBoxHeader(box);
  uint32_t v = readU8();
  box.offsetSize = static_cast<uint8_t>(v >> 4u);
  if(!(box.offsetSize == 0 || box.offsetSize == 4 || box.offsetSize == 8)) {
    return std::make_optional(tinyformat::format("Invalid ItemLocationBox::offsetSize=%d", box.offsetSize));
  }
  box.lengthSize = static_cast<uint8_t>(v & 0xfu);
  if(!(box.lengthSize == 0 || box.lengthSize == 4 || box.lengthSize == 8)) {
    return std::make_optional(tinyformat::format("Invalid ItemLocationBox::lengthSize=%d", box.offsetSize));
  }
  v = readU8();
  box.baseOffsetSize = static_cast<uint8_t>(v >> 4u);
  if(!(box.baseOffsetSize == 0 || box.baseOffsetSize == 4 || box.baseOffsetSize == 8)) {
    return std::make_optional(tinyformat::format("Invalid ItemLocationBox::baseOffsetSize=%d", box.offsetSize));
  }
  if(box.version() == 1 || box.version() == 2) {
    box.indexSize = static_cast<uint8_t>(v & 0xfu);
    if(!(box.indexSize == 0 || box.indexSize == 4 || box.indexSize == 8)) {
      return std::make_optional(tinyformat::format("Invalid ItemLocationBox::baseOffsetSize=%d", box.offsetSize));
    }
  }
  if(box.version() < 2) {
    box.itemCount = readU16();
  }else if(box.version() == 2){
    box.itemCount = readU32();
  }else{
    return std::make_optional(tinyformat::format("Unknwon ItemLocationBox version=%d", box.version()));
  }
  for(uint32_t i = 0; i < box.itemCount; ++i) {
    ItemLocationBox::Item item{};
    if (box.version() < 2) {
      item.itemID = readU16();
    } else if (box.version() == 2) {
      item.itemID = readU32();
    } else {
      return std::make_optional(tinyformat::format("Unknwon ItemLocationBox version=%d", box.version()));
    }
    if (box.version() == 1 || box.version() == 2) {
      item.constructionMethod = readU16() & 0x7u;
    }
    item.dataReferenceIndex = readU16();
    item.baseOffset = readUint(box.baseOffsetSize).value();
    item.extentCount = readU16();
    for (uint32_t j = 0; j < item.extentCount; ++j) {
      ItemLocationBox::Item::Extent extent{};
      if((box.version() == 1 || box.version() == 2) && (box.indexSize > 0)) {
        extent.extentIndex = readUint(box.indexSize).value();
      }
      extent.extentOffset = readUint(box.offsetSize).value();
      extent.extentLength = readUint(box.lengthSize).value();

      item.extents.emplace_back(extent);
    }
    box.items.emplace_back(item);
  }
  return std::optional<std::string>();
}

std::optional<std::string> Parser::parseMediaDataBox(MediaDataBox& box, size_t const end) {
  box.beg = this->pos_;
  box.end = end;
  return std::optional<std::string>();
}

//-----------------------------------------------------------------------------
// util
//-----------------------------------------------------------------------------

void Parser::parseFullBoxHeader(FullBox& fullBox) {
  // See: ISOBMFF p.22
  uint32_t v = readU32();
  auto const version = static_cast<uint8_t>(v >> 24u);
  auto const flags = (v & 0x00ffffffu);
  fullBox.setFullBoxHeader(version, flags);
}

}