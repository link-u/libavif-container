//
// Created by psi on 2020/01/12.
//

#include "Writer.hpp"
#include "util/FourCC.hpp"

using avif::util::str2uint;
using avif::util::uint2str;

namespace avif {

Writer::Writer(util::Logger& log, util::StreamWriter& writer)
:log_(log)
,stream_(writer)
{

}

//-----------------------------------------------------------------------------
// util
//-----------------------------------------------------------------------------

Writer::BoxContext Writer::beginBoxHeader(const char type[4], Box &box) {
  box.hdr.offset = stream_.size();
  box.hdr.type = str2uint(type);
  this->putU32(box.hdr.size);
  this->putU32(box.hdr.type);
  return Writer::BoxContext(this, box);
}

Writer::BoxContext Writer::beginFullBoxHeader(const char type[4], FullBox& box) {
  // box header
  box.hdr.offset = stream_.size();
  box.hdr.type = str2uint(type);
  this->putU32(box.hdr.size);
  this->putU32(box.hdr.type);
  // full box header
  this->putU32(static_cast<uint32_t>(box.version() << 24u) | box.flags());
  return Writer::BoxContext(this, box);
}

void Writer::putTypeString(std::string const& type) {
  if(type.size() != 4) {
    throw std::out_of_range(tfm::format("The length of type string must be 4, got %s", type));
  }
  this->putU32(str2uint(type.c_str()));
}

void Writer::putString(std::string const& str) {
  this->stream_.append(reinterpret_cast<uint8_t const*>(str.data()), str.size());
  this->stream_.putU8(0);
}

//-----------------------------------------------------------------------------
// Box Context
//-----------------------------------------------------------------------------

Writer::BoxContext::BoxContext(Writer* parent, Box &box)
:parent_(parent)
,box_(box)
{
}

Writer::BoxContext::BoxContext(Writer::BoxContext&& ctx) noexcept
:parent_(ctx.parent_)
,box_(ctx.box_)
{
  ctx.parent_ = nullptr;
}

Writer::BoxContext::~BoxContext() noexcept {
  if(this->parent_) {
    this->box_.hdr.size = this->parent_->stream_.size() - this->box_.hdr.offset;
  }
}

//-----------------------------------------------------------------------------
// write functions
//-----------------------------------------------------------------------------

void Writer::write(FileBox& fileBox) {
  this->writeFileTypeBox(fileBox.fileTypeBox);
  this->writeMetaBox(fileBox.metaBox);
  for (auto& mdat : fileBox.mediaDataBoxes) {
    this->writeMediaDataBox(mdat);
  }
}

void Writer::writeFileTypeBox(FileTypeBox& box) {
  auto context = this->beginBoxHeader("ftyp", box);
  this->putTypeString(box.majorBrand);
  this->putU32(box.minorVersion);
  for(std::string const& brand : box.compatibleBrands) {
    this->putTypeString(brand);
  }
}

void Writer::writeMetaBox(MetaBox& box) {
  auto context = this->beginFullBoxHeader("meta", box);
  this->writeHandlerBox(box.handlerBox);
  this->writeItemPropertiesBox(box.itemPropertiesBox);
  this->writeItemInfoBox(box.itemInfoBox);
  this->writeItemLocationBox(box.itemLocationBox);
  if(box.primaryItemBox.has_value()) {
    this->writePrimaryItemBox(box.primaryItemBox.value());
  }
}

void Writer::writeHandlerBox(HandlerBox& box) {
  auto context = this->beginFullBoxHeader("hdlr", box);
  this->putU32(0 /* reserved*/);
  this->putTypeString(box.handler);
  this->putU32(0 /* reserved*/);
  this->putU32(0 /* reserved*/);
  this->putU32(0 /* reserved*/);
  this->putString(box.name);
}

void Writer::writeItemPropertiesBox(ItemPropertiesBox& box) {
  auto context = this->beginBoxHeader("iprp", box);
  this->writeItemPropertyContainer(box.itemPropertyContainer);
  for(auto& ipma : box.itemPropertyAssociations) {
    this->writeItemPropertyAssociation(ipma);
  }
}

void Writer::writeItemPropertyContainer(ItemPropertyContainer& box) {
  auto context = this->beginBoxHeader("ipco", box);
  for (auto& prop : box.properties) {
    if (std::holds_alternative<PixelAspectRatioBox>(prop)) {
      this->writePixelAspectRatioBox(std::get<PixelAspectRatioBox>(prop));
    } else if (std::holds_alternative<ImageSpatialExtentsProperty>(prop)) {
      this->writeImageSpatialExtentsProperty(std::get<ImageSpatialExtentsProperty>(prop));
    } else if (std::holds_alternative<PixelInformationProperty>(prop)) {
      this->writePixelInformationProperty(std::get<PixelInformationProperty>(prop));
    } else if (std::holds_alternative<CleanApertureBox>(prop)) {
      this->writeCleanApertureBox(std::get<CleanApertureBox>(prop));
    } else if (std::holds_alternative<AV1CodecConfigurationRecordBox>(prop)) {
      this->writeAV1CodecConfigurationRecordBox(std::get<AV1CodecConfigurationRecordBox>(prop));
    } else {
      throw std::logic_error(tfm::format("Unknown box type: %s, idx=%d", typeid(prop).name(), prop.index()));
    }
  }
}

void Writer::writePixelAspectRatioBox(PixelAspectRatioBox& box) {
  auto context = this->beginBoxHeader("pasp", box);
  putU32(box.hSpacing);
  putU32(box.vSpacing);
}

void Writer::writeImageSpatialExtentsProperty(ImageSpatialExtentsProperty& box) {
  auto context = this->beginFullBoxHeader("ispe", box);
  putU32(box.imageWidth);
  putU32(box.imageHeight);
}

void Writer::writePixelInformationProperty(PixelInformationProperty& box) {
  auto context = this->beginFullBoxHeader("pixi", box);
  this->putU8(box.bitsPerChannel.size());
  for (auto& bpp : box.bitsPerChannel) {
    this->putU8(bpp);
  }
}

void Writer::writeCleanApertureBox(CleanApertureBox& box) {
  putU32(box.cleanApertureWidthN);
  putU32(box.cleanApertureWidthD);
  putU32(box.cleanApertureHeightN);
  putU32(box.cleanApertureHeightD);
  putU32(box.horizOffN);
  putU32(box.horizOffD);
  putU32(box.vertOffN);
  putU32(box.vertOffD);
}

void Writer::writeAV1CodecConfigurationRecordBox(AV1CodecConfigurationRecordBox& box) {
  auto context = this->beginBoxHeader("av1C", box);
  AV1CodecConfigurationRecord& conf = box.av1Config;
  putU8((conf.marker ? 0x80u : 0x00u) | conf.version);
  putU8(static_cast<uint8_t>(conf.seqProfile << 5u) | conf.seqLevelIdx0);
  putU8(
      static_cast<uint8_t>(conf.seqTier0 << 7u) |
      (conf.highBitDepth ? 1u << 6u : 0u) |
      (conf.twelveBit ? 1u << 5u : 0u) |
      (conf.monochrome ? 1u << 4u : 0u) |
      static_cast<uint8_t>(conf.chromaSubsamplingY << 3u) |
      static_cast<uint8_t>(conf.chromaSubsamplingY << 2u) |
      (conf.chromaSamplePosition & 3u));
  bool initialPresentationDelayPresent = conf.initialPresentationDelayPresent && conf.initialPresentationDelay > 0;
  if (initialPresentationDelayPresent) {
    putU8((1u << 4u) | ((conf.initialPresentationDelay & 15u) - 1));
  } else {
    putU8(0u);
  }
  append(conf.configOBUs);
}


void Writer::writeItemPropertyAssociation(ItemPropertyAssociation& box) {
  auto context = this->beginFullBoxHeader("ipma", box);
  this->putU32(box.items.size());
  for (auto& item : box.items) {
    if (box.version() < 1) {
      putU16(item.itemID);
    } else {
      putU32(item.itemID);
    }
    putU8(item.entries.size());
    for (auto const& ent : item.entries) {
      if ((box.flags() & 1u) == 1u) {
        putU16((ent.essential ? 0x8000u : 0x0) | ent.propertyIndex);
      } else {
        putU8((ent.essential ? 0x80u : 0x0) | ent.propertyIndex);
      }
    }
  }
}

void Writer::writeItemInfoBox(ItemInfoBox& box) {
  auto context = this->beginFullBoxHeader("iinf", box);
  if (box.version() == 0) {
    putU16(box.itemInfos.size());
  } else {
    putU32(box.itemInfos.size());
  }
  for (auto& item : box.itemInfos) {
    this->writeItemInfoEntry(item);
  }
}

void Writer::writeItemInfoEntry(ItemInfoEntry& box) {
  auto context = this->beginFullBoxHeader("infe", box);
  if (box.version() == 0 || box.version() == 1) {
    putU16(box.itemID);
    putU16(box.itemProtectionIndex);
    putString(box.itemName);
    putString(box.contentType);
    putString(box.contentEncoding.has_value() ? box.contentEncoding.value() : "");
  }
  if (box.version() == 1) {
    if (std::holds_alternative<FDItemInfoExtension>(box.itemInfoExtension)) {
      auto& ext = std::get<FDItemInfoExtension>(box.itemInfoExtension);
      ext.extensionType = str2uint("fdel");
      putU32(ext.extensionType);
      putString(ext.contentLocation);
      putU64(ext.contentLength);
      putU64(ext.transferLength);
      putU8(ext.groupIDs.size());
      for (auto& id : ext.groupIDs) {
        putU8(id);
      }
    }
  }
  if(box.version() >= 2) {
    if (box.version() == 2) {
      putU16(box.itemID);
    } else if(box.version() == 3) {
      putU32(box.itemID);
    } else {
      throw std::runtime_error(tfm::format("ItemInfoEntry with version=%d not supported.", box.version()));
    }
    putU16(box.itemProtectionIndex);
    uint32_t const itemType = str2uint(box.itemType.value().c_str());
    putU32(itemType);
    putString(box.itemName);
    switch(itemType) {
      case str2uint("mime"):
        putString(box.contentType);
        putString(box.contentEncoding.value());
        break;
      case str2uint("uri "):
        putString(box.itemURIType.value());
        break;
      default:
        break;
    }
  }
}

void Writer::writeItemLocationBox(ItemLocationBox& box) {
  auto context = this->beginFullBoxHeader("iloc", box);
  putU8(static_cast<uint8_t>(box.offsetSize << 4u) | box.lengthSize);
  putU8(static_cast<uint8_t>(box.baseOffsetSize << 4u) | (box.version() == 1u || box.version() == 2 ? box.indexSize : 0u));
  if (box.version() < 2) {
    putU16(box.items.size());
  } else if(box.version() == 2) {
    putU32(box.items.size());
  } else {
    throw std::runtime_error(tfm::format("Unknwon ItemLocationBox version=%d", box.version()));
  }
  for (auto& item : box.items) {
    if (box.version() < 2) {
      putU16(item.itemID);
    } else if(box.version() == 2) {
      putU32(item.itemID);
    } else {
      throw std::runtime_error(tfm::format("Unknwon ItemLocationBox version=%d", box.version()));
    }
    if (box.version() == 1 || box.version() == 2) {
      putU16(item.constructionMethod);
    }
    putU16(item.dataReferenceIndex);
    switch (box.baseOffsetSize) {
      case 0:
        break;
      case 4:
        putU32(item.baseOffset);
        break;
      case 8:
        putU64(item.baseOffset);
        break;
      default:
        throw std::runtime_error(tfm::format("Illegal base offset size=%d", box.baseOffsetSize));
    }
    putU16(item.extents.size());
    for (auto& extent : item.extents) {
      if((box.version() == 1 || box.version() == 2) && (box.indexSize > 0)) {
        switch (box.indexSize) {
          case 0:
            break;
          case 4:
            putU32(extent.extentIndex);
            break;
          case 8:
            putU64(extent.extentIndex);
            break;
          default:
            throw std::runtime_error(tfm::format("Illegal index size=%d", box.indexSize));
        }
      }
      switch (box.offsetSize) {
        case 0:
          break;
        case 4:
          putU32(extent.extentOffset);
          break;
        case 8:
          putU64(extent.extentOffset);
          break;
        default:
          throw std::runtime_error(tfm::format("Illegal offset size=%d", box.offsetSize));
      }
      switch (box.lengthSize) {
        case 0:
          break;
        case 4:
          putU32(extent.extentLength);
          break;
        case 8:
          putU64(extent.extentLength);
          break;
        default:
          throw std::runtime_error(tfm::format("Illegal length size=%d", box.lengthSize));
      }
    }
  }
}

void Writer::writePrimaryItemBox(PrimaryItemBox& box) {
  if(box.version() == 0){
    putU16(box.itemID);
  } else {
    putU32(box.itemID);
  }
}

void Writer::writeMediaDataBox(MediaDataBox& box) {
  auto context = this->beginBoxHeader("mdat", box);
  std::vector<uint8_t> dummy;
  box.offset = this->stream_.size();
  dummy.resize(box.size);
  append(dummy);
}

}
