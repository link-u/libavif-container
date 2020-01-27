//
// Created by psi on 2020/01/12.
//

#pragma once

#include "util/Logger.hpp"
#include "util/StreamWriter.hpp"
#include "FileBox.hpp"
#include "ItemReferenceBox.hpp"

namespace avif {

class Writer {
private:
  class BoxContext {
    Writer* parent_;
    Box& box_;
  public:
    explicit BoxContext(Writer* parent, Box& box);
    BoxContext(BoxContext const&) = delete;
    BoxContext& operator=(BoxContext const&) = delete;
    BoxContext(BoxContext&& ctx) noexcept;
    BoxContext& operator=(BoxContext&& ctx) = delete;
    ~BoxContext() noexcept;
  };
private:
  util::Logger& log_;
  util::StreamWriter& stream_;
public:
  Writer() = delete;
  Writer(Writer const&) = delete;
  Writer(Writer&&) = delete;
  Writer& operator=(Writer const&) = delete;
  Writer& operator=(Writer&&) = delete;
  explicit Writer(util::Logger& log, util::StreamWriter& writer);

public:
  void write(FileBox& fileBox);

private:
  BoxContext beginBoxHeader(const char type[4], Box& box);
  BoxContext beginFullBoxHeader(const char type[4], FullBox& box);

private:
  void putU8(uint8_t const data) { this->stream_.putU8(data); }
  void putU16(uint16_t const data) { this->stream_.putU16B(data); };
  void putU32(uint32_t const data) { this->stream_.putU32B(data); };
  void putU64(uint64_t const data) { this->stream_.putU64B(data); };
  void append(std::vector<uint8_t> const& data) { this->stream_.append(data); };
  void append(uint8_t const*const data, size_t const length) { this->stream_.append(data, length); };
  void putTypeString(std::string const& type);
  void putString(std::string const& str);

private:
  void writeFileTypeBox(FileTypeBox& box);

  void writeMetaBox(MetaBox& box);

  void writeHandlerBox(HandlerBox& box);

  void writeItemPropertiesBox(ItemPropertiesBox& box);
  void writeItemPropertyContainer(ItemPropertyContainer& box);
  void writePixelAspectRatioBox(PixelAspectRatioBox& box);
  void writeImageSpatialExtentsProperty(ImageSpatialExtentsProperty& box);
  void writePixelInformationProperty(PixelInformationProperty& box);
  void writeRelativeLocationProperty(RelativeLocationProperty& rloc);
  void writeAuxiliaryTypeProperty(AuxiliaryTypeProperty& aux);
  void writeCleanApertureBox(CleanApertureBox &box);
  void writeImageRotationBox(ImageRotationBox &box);
  void writeImageMirrorBox(ImageMirrorBox &box);
  void writeColourInformationBox(ColourInformationBox& box);
  void writeContentLightLevelBox(ContentLightLevelBox& box);
  void writeMasteringDisplayColourVolumeBox(MasteringDisplayColourVolumeBox& box);
  void writeAV1CodecConfigurationRecordBox(AV1CodecConfigurationRecordBox& box);
  void writeItemPropertyAssociation(ItemPropertyAssociation& box);

  void writeItemInfoBox(ItemInfoBox& box);
  void writeItemInfoEntry(ItemInfoEntry& box);
  void writeItemLocationBox(ItemLocationBox& box);
  void writePrimaryItemBox(PrimaryItemBox& box);
  void writeItemReferenceBox(ItemReferenceBox& box);

  void writeMediaDataBox(MediaDataBox& box);
};

}


