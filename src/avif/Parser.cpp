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
#include "ItemInfoExtension.hpp"
#include "util/FourCC.hpp"
#include "PrimaryItemBox.hpp"

using avif::util::str2uint;
using avif::util::uint2str;

namespace avif {

constexpr uint32_t boxType(const char *str) {
  return str2uint(str);
}
//-----------------------------------------------------------------------------

Parser::Parser(util::Logger& log, std::vector<uint8_t> buff)
:log_(log)
,buffer_(std::move(buff))
,reader_(log, buffer_)
,fileBox_()
{
}

std::shared_ptr<Parser::Result> Parser::parse() {
  if(this->result_) {
    return this->result_;
  }
  try {
    this->parseFile();
    this->result_ = std::make_shared<Parser::Result>(std::move(this->buffer_), std::move(fileBox_));
  } catch(Parser::Error& err) {
    this->result_ = std::make_shared<Parser::Result>(std::move(this->buffer_), std::move(err));
  } catch(std::exception& err) {
    this->result_ = std::make_shared<Parser::Result>(std::move(this->buffer_), Parser::Error(err));
  } catch(...) {
    this->result_ = std::make_shared<Parser::Result>(std::move(this->buffer_), Parser::Error("Unknwon error"));
  }
  return this->result_;
}

void Parser::parseFile() {
  while(!this->consumed()) {
    this->parseBoxInFile();
  }
}

void Parser::parseBoxInFile() {
  FileBox& box = this->fileBox_;
  Box::Header hdr = readBoxHeader();
  switch(hdr.type) {
    case boxType("ftyp"):
      // ISO/IEC 14496-12:2015(E)
      // 4.3 File Type Box
      // Quantity: Exactly one (but see below)
      box.fileTypeBox.hdr = hdr;
      this->parseFileTypeBox(box.fileTypeBox, hdr.end());
      break;
    case boxType("free"):
    case boxType("skip"):
      // ISO/IEC 14496-12:2015(E)
      // 8.1.2
      // free_type may be ‘free’ or ‘skip’.
      break;
    case boxType("meta"): {
      // ISO/IEC 14496-12:2015(E)
      // 8.11.1
      // Quantity: Zero or one (in File, ‘moov’, and ‘trak’), One or more (in ‘meco’)
      box.metaBox.hdr = hdr;
      this->parseMetaBox(box.metaBox, hdr.end());
      break;
    }
    case boxType("mdat"): {
      // ISO/IEC 14496-12:2015(E)
      // 8.1.1. Media Data Box
      // Quantity: Zero or more
      MediaDataBox mediaDataBox{};
      mediaDataBox.hdr = hdr;
      this->parseMediaDataBox(mediaDataBox, hdr.end());
      box.mediaDataBoxes.emplace_back(mediaDataBox);
      break;
    }
    default:
      warningUnknownBox(hdr);
      break;
  }
  this->seek(hdr.end());
}

void Parser::parseFileTypeBox(FileTypeBox& box, size_t const end) {
  uint32_t const majorBrand = readU32();
  uint32_t const minorVersion = readU32();
  if(str2uint("avif") != majorBrand) {
    throw Error("Unsupported brand: %s", uint2str(majorBrand));
  }
  if(minorVersion != 0) {
    throw Error("We currently just support version 0(!=%d)", minorVersion);
  }
  std::vector<std::string> compatibleBrands;
  while(this->pos() < end) {
    uint32_t const compatBrand = readU32();
    compatibleBrands.emplace_back(uint2str(compatBrand));
  }
  box.majorBrand = uint2str(majorBrand);
  box.minorVersion = minorVersion;
  box.compatibleBrands = std::move(compatibleBrands);
}

// See ISOBMFF p.90
void Parser::parseMetaBox(MetaBox& box, size_t const end) {
  // MetaBox extends FullBox.
  // See: ISOBMFF p.21
  this->parseFullBoxHeader(box);
  while(this->pos() < end) {
    this->parseBoxInMeta(box, end);
  }
}

void Parser::parseBoxInMeta(MetaBox& box, size_t const endOfBox) {
  Box::Header const hdr = readBoxHeader();
  switch(hdr.type) {
    case boxType("hdlr"):
      box.handlerBox.hdr = hdr;
      this->parseHandlerBox(box.handlerBox, hdr.end());
      break;
    case boxType("iprp"):
      // ISO/IEC 23008-12:2017 p.28
      // 9.3 Item Properties Box
      box.itemPropertiesBox.hdr = hdr;
      this->parseItemPropertiesBox(box.itemPropertiesBox, hdr.end());
      break;
    case boxType("iinf"):
      // 8.11.6 Item Information Box
      // See: ISOBMFF p.95
      box.itemInfoBox.hdr = hdr;
      this->parseItemInfoBox(box.itemInfoBox, hdr.end());
      break;
    case boxType("iloc"):
      // 8.11.3 The Item Location Box
      // See: ISOBMFF p.91
      box.itemLocationBox.hdr = hdr;
      this->parseItemLocationBox(box.itemLocationBox, hdr.end());
      break;
    case boxType("pitm"): {
      // 8.11.3 The Item Location Box
      // See: ISOBMFF p.80
      PrimaryItemBox pitm{};
      this->parsePrimaryItemBox(pitm, hdr.end());
      box.primaryItemBox = pitm;
      break;
    }
    default:
      warningUnknownBox(hdr);
      break;
  }
  // ISO/IEC 23008-12:2017 p.34
  // 10.2 Image and image collection brands
  // Note particularly that the brand 'mif1' does not mandate a MovieBox ('moov').
  this->seek(hdr.end());
}

void Parser::parseHandlerBox(HandlerBox& box, size_t const end) {
  // HandlerBox extends FullBox.
  // See: ISOBMFF p.43
  this->parseFullBoxHeader(box);

  // ISOBMFF p.44 8.4.3.2
  uint32_t reserved;
  reserved = readU32(); // pre_defined = 0
  uint32_t const handlerType = readU32();
  reserved = readU32(); // reserved
  reserved = readU32(); // reserved
  reserved = readU32(); // reserved
  box.name = readString();
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
}

void Parser::parseItemPropertiesBox(ItemPropertiesBox& box, size_t const end) {
  // ISO/IEC 23008-12:2017 p.28
  // 9.3 Item Properties Box
  {
    Box::Header const hdr = readBoxHeader();
    if(hdr.type != str2uint("ipco")) {
      throw Error("ItemPropertyContainer expected, got %s", uint2str(hdr.type));
    }
    box.itemPropertyContainer.hdr = hdr;
    while(this->pos() < hdr.end()) {
      this->parseBoxInItemPropertyContainer(box.itemPropertyContainer);
    }
    this->seek(hdr.end());
  }
  // read "ipma"s
  while(this->pos() < end) {
    Box::Header const hdr = readBoxHeader();
    if(hdr.type != str2uint("ipma")) {
      throw Error("ItemPropertyAssociation(ipma) expected, got %s", uint2str(hdr.type));
    }
    ItemPropertyAssociation itemPropertyAssociation{};
    itemPropertyAssociation.hdr = hdr;
    this->parseItemPropertyAssociation(itemPropertyAssociation);
    box.itemPropertyAssociations.emplace_back(itemPropertyAssociation);
    this->seek(hdr.end());
  }
}

void Parser::parseBoxInItemPropertyContainer(ItemPropertyContainer& container) {
  // https://github.com/nokiatech/heif/blob/master/srcs/common/itempropertycontainer.cpp
  Box::Header const hdr = readBoxHeader();
  switch(hdr.type) {
    case boxType("pasp"): {
      PixelAspectRatioBox box{};
      box.hdr = hdr;
      this->parsePixelAspectRatioBox(box, hdr.end());
      container.properties.emplace_back(box);
      break;
    }
    case boxType("ispe"): {
      ImageSpatialExtentsProperty box{};
      box.hdr = hdr;
      this->parseImageSpatialExtentsProperty(box, hdr.end());
      container.properties.emplace_back(box);
      break;
    }
    case boxType("pixi"): {
      // ISO/IEC 23008-12:2017(E)
      // p.13
      // 6.5.6.1 Pixel information
      PixelInformationProperty box{};
      box.hdr = hdr;
      this->parsePixelInformationProperty(box, hdr.end());
      container.properties.emplace_back(box);
      break;
    }
    case boxType("clap"): {
      // ISO/IEC 14496-12:2015(E)
      // p.158
      CleanApertureBox box{};
      box.hdr = hdr;
      this->parseCleanApertureBox(box, hdr.end());
      container.properties.emplace_back(box);
      break;
    }
    case boxType("av1C"): {
      AV1CodecConfigurationRecordBox box{};
      box.hdr = hdr;
      this->parseAV1CodecConfigurationRecordBox(box, hdr.end());
      container.properties.emplace_back(box);
      break;
    }
    case boxType("free"):
    case boxType("skip"):
      // ISO/IEC 14496-12:2015(E)
      // 8.1.2
      // free_type may be ‘free’ or ‘skip’.
      break;
    default:
      warningUnknownBox(hdr);
      break;
  }
  this->seek(hdr.end());
}

void Parser::parsePixelAspectRatioBox(PixelAspectRatioBox& box, size_t const end) {
  // 12.1.4 Pixel Aspect Ratio and Clean Aperture
  // See: ISOBMFF p.170
  box.hSpacing = readU32();
  box.vSpacing = readU32();
}

void Parser::parseImageSpatialExtentsProperty(ImageSpatialExtentsProperty& prop, size_t const end) {
  // ISO/IEC 23008-12:2017(E)
  // p.11
  // 6.5.3 Image spatial extents
  parseFullBoxHeader(prop);
  prop.imageWidth = readU32();
  prop.imageHeight = readU32();
}

void Parser::parsePixelInformationProperty(PixelInformationProperty& prop, size_t const end) {
  // ISO/IEC 23008-12:2017(E)
  // p.13
  // 6.5.6.1 Pixel information
  parseFullBoxHeader(prop);
  uint8_t const numChannels = readU8();
  for(uint8_t i =0; i < numChannels; ++i) {
    prop.bitsPerChannel.emplace_back(readU8());
  }
}

void Parser::parseCleanApertureBox(CleanApertureBox& box, size_t end) {
  // ISO/IEC 14496-12:2015(E)
  // p.158

  // a fractional number which defines the exact
  // clean aperture width, in counted pixels, of the video image
  box.cleanApertureWidthN = readU32();
  box.cleanApertureWidthD = readU32();

  // a fractional number which defines the
  // exact clean aperture height, in counted pixels, of the video image
  box.cleanApertureHeightN = readU32();
  box.cleanApertureHeightD = readU32();

  // a fractional number which defines the horizontal offset of clean
  // aperture centre minus (width‐1)/2. Typically 0.
  box.horizOffN = readU32();
  box.horizOffD = readU32();

  // a fractional number which defines the vertical offset of clean aperture
  // centre minus (height‐1)/2. Typically 0.
  box.vertOffN = readU32();
  box.vertOffD = readU32();
}

void Parser::parseAV1CodecConfigurationRecordBox(AV1CodecConfigurationRecordBox& box, size_t const end) {
// https://aomediacodec.github.io/av1-isobmff/#av1codecconfigurationbox-section
 AV1CodecConfigurationRecord& conf = box.av1Config;
  uint8_t tmp = readU8();
  conf.marker = (tmp & 0x80u) == 0x80u;
  conf.version = tmp & 0x7fu;
  tmp = readU8();
  conf.seqProfile = static_cast<uint8_t>(tmp >> 5u) & 0x7u;
  conf.seqLevelIdx0 = tmp & 31u;
  tmp = readU8();
  conf.seqTier0 =             static_cast<uint8_t>(tmp >> 7u) & 1u;
  conf.highBitDepth =         (tmp & (1u << 6u)) == (1u << 6u);
  conf.twelveBit =            (tmp & (1u << 5u)) == (1u << 5u);
  conf.monochrome =           (tmp & (1u << 4u)) == (1u << 4u);
  conf.chromaSubsamplingX =   static_cast<uint8_t>(tmp >> 3u) & 1u;
  conf.chromaSubsamplingY =   static_cast<uint8_t>(tmp >> 2u) & 1u;
  conf.chromaSamplePosition = tmp & 3u;
  tmp = readU8();
  tmp = tmp & 31u; // unsigned int (3) reserved = 0;
  conf.initialPresentationDelayPresent = (tmp & (1u << 4u)) == (1u << 4u);
  if(conf.initialPresentationDelayPresent) {
    conf.initialPresentationDelay = (tmp & 15u) + 1u;
  } else {
    conf.initialPresentationDelay = 0;
  }
  conf.configOBUs = std::vector<uint8_t>(std::next(this->buffer_.begin(), this->pos()), std::next(this->buffer_.begin(), end));
}

void Parser::parseItemPropertyAssociation(ItemPropertyAssociation& assoc) {
  // https://github.com/nokiatech/heif/blob/master/srcs/common/itempropertyassociation.cpp
  parseFullBoxHeader(assoc);
  uint32_t const itemCount = readU32();
  for(uint32_t i = 0; i < itemCount; ++i) {
    ItemPropertyAssociation::Item item;
    if(assoc.version() < 1) {
      item.itemID = readU16();
    } else {
      item.itemID = readU32();
    }
    uint8_t entryCount = readU8();
    for(uint8_t j = 0; j < entryCount; ++j) {
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
}

void Parser::parseItemInfoBox(ItemInfoBox& box, size_t const end) {
  // 8.11.6 Item Information Box
  // See: ISOBMFF p.95
  this->parseFullBoxHeader(box);
  uint32_t entryCount;
  if(box.version() == 0){
    entryCount = readU16();
  } else {
    entryCount = readU32();
  }
  for(uint32_t i = 0; i < entryCount; ++i) {
    Box::Header hdr = readBoxHeader();
    if(hdr.type != str2uint("infe")) {
      throw Error("'infe' expected in ItemInfoBox, got %s", uint2str(hdr.type));
    }
    ItemInfoEntry entry;
    entry.hdr = hdr;
    this->parseItemInfoEntry(entry, hdr.end());
    box.itemInfos.emplace_back(std::move(entry));
    this->seek(hdr.end());
  }
}

void Parser::parseItemInfoEntry(ItemInfoEntry& box, size_t const end) {
  parseFullBoxHeader(box);
  if(box.version() == 0 || box.version() == 1) {
    box.itemID = readU16();
    box.itemProtectionIndex = readU16();
    box.itemName = readString();
    box.contentType = readString();
    box.contentEncoding = readString();
  }
  if(box.version() == 1) {
    uint32_t const extensionType = readU32();
    switch (extensionType) {
    case str2uint("fdel"): {
      FDItemInfoExtension ext;
      ext.extensionType = extensionType;
      ext.contentLocation = readString();
      ext.contentLength = readU64();
      ext.transferLength = readU64();
      uint8_t entryCount = readU8();
      for(uint8_t i = 0; i < entryCount; ++i) {
        ext.groupIDs.emplace_back(readU8());
      }
      std::get<FDItemInfoExtension>(box.itemInfoExtension) = std::move(ext);
      break;
    }
      default:
        throw Error("Unknwon ItemInfoExtension with type = %s", uint2str(extensionType));
    }
  }
  if(box.version() >= 2) {
    if (box.version() == 2) {
      box.itemID = readU16();
    } else if(box.version() == 3) {
      box.itemID = readU32();
    } else {
      throw Error("ItemInfoEntry with version=%d not supported.", box.version());
    }
    box.itemProtectionIndex = readU16();
    uint32_t const itemType = readU32();
    box.itemType = std::make_optional<std::string>(uint2str(itemType));
    box.itemName = readString();
    switch(itemType) { // There may be additional data in particular cases.
      case str2uint("mime"):
        box.contentType = readString();
        box.contentEncoding = readString();
        break;
      case str2uint("uri "):
        box.itemURIType = std::make_optional<std::string>(readString());
        break;
      default:
        break;
    }
  }
}

void Parser::parseItemLocationBox(ItemLocationBox& box, size_t const end) {
  // 8.11.3 The Item Location Box
  // See: ISOBMFF p.91
  this->parseFullBoxHeader(box);
  uint32_t v = readU8();
  box.offsetSize = static_cast<uint8_t>(v >> 4u);
  if(!(box.offsetSize == 0 || box.offsetSize == 4 || box.offsetSize == 8)) {
    throw Error("Invalid ItemLocationBox::offsetSize=%d", box.offsetSize);
  }
  box.lengthSize = static_cast<uint8_t>(v & 0xfu);
  if(!(box.lengthSize == 0 || box.lengthSize == 4 || box.lengthSize == 8)) {
    throw Error("Invalid ItemLocationBox::lengthSize=%d", box.offsetSize);
  }
  v = readU8();
  box.baseOffsetSize = static_cast<uint8_t>(v >> 4u);
  if(!(box.baseOffsetSize == 0 || box.baseOffsetSize == 4 || box.baseOffsetSize == 8)) {
    throw Error("Invalid ItemLocationBox::baseOffsetSize=%d", box.offsetSize);
  }
  if(box.version() == 1 || box.version() == 2) {
    box.indexSize = static_cast<uint8_t>(v & 0xfu);
    if(!(box.indexSize == 0 || box.indexSize == 4 || box.indexSize == 8)) {
      throw Error("Invalid ItemLocationBox::baseOffsetSize=%d", box.offsetSize);
    }
  }
  uint32_t itemCount = 0;
  if(box.version() < 2) {
    itemCount = readU16();
  }else if(box.version() == 2){
    itemCount = readU32();
  }else{
    throw Error("Unknwon ItemLocationBox version=%d", box.version());
  }
  for(uint32_t i = 0; i < itemCount; ++i) {
    ItemLocationBox::Item item{};
    if (box.version() < 2) {
      item.itemID = readU16();
    } else if (box.version() == 2) {
      item.itemID = readU32();
    } else {
      throw Error("Unknwon ItemLocationBox version=%d", box.version());
    }
    if (box.version() == 1 || box.version() == 2) {
      item.constructionMethod = readU16() & 0x7u;
    }
    item.dataReferenceIndex = readU16();
    item.baseOffset = readUint(box.baseOffsetSize).value();
    uint16_t const extentCount = readU16();
    for (uint32_t j = 0; j < extentCount; ++j) {
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
}

void Parser::parsePrimaryItemBox(avif::PrimaryItemBox& box, size_t end) {
  parseFullBoxHeader(box);
  if(box.version() == 0) {
    box.itemID = readU16();
  } else {
    box.itemID = readU32();
  }
}

void Parser::parseMediaDataBox(MediaDataBox& box, size_t const end) {
  box.offset = this->pos();
  box.size = end - box.offset;
}

//-----------------------------------------------------------------------------
// util
//-----------------------------------------------------------------------------

Box::Header Parser::readBoxHeader() {
  Box::Header hdr{};
  hdr.offset = this->pos();
  hdr.size = readU32();
  hdr.type = readU32();
  if(hdr.size == 1) {
    throw Error("LargeSize box is not supported.");
  }
  if(hdr.size == 0) {
    hdr.size = this->buffer_.size() - hdr.offset;
  }
  if((hdr.end()) > this->buffer_.size()) {
    throw Error("File corrupted. Detected at %s box, from %d to %d, but buffer.size = %d.", uint2str(hdr.type), hdr.offset, hdr.end(), this->buffer_.size());
  }
  return hdr;
}

void Parser::parseFullBoxHeader(FullBox& fullBox) {
  // See: ISOBMFF p.22
  uint32_t v = readU32();
  auto const version = static_cast<uint8_t>(v >> 24u);
  auto const flags = (v & 0x00ffffffu);
  fullBox.setFullBoxHeader(version, flags);
}

void Parser::warningUnknownBox(Box::Header const& hdr) {
  std::string typeStr = uint2str(hdr.type);
  log().warn("Unknown box type=%s(=0x%0lx) with size=%ld(%ld~%ld/%ld)", typeStr.c_str(), hdr.type, hdr.size, hdr.offset, hdr.end(), this->buffer_.size());
}

}