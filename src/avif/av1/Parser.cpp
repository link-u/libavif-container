//
// Created by psi on 2020/01/05.
//

#include "Parser.hpp"

namespace avif::av1 {

Parser::Parser(util::Logger& log, std::vector<uint8_t> buffer)
:log_(log)
,buffer_(std::move(buffer))
,reader_(log, buffer_)
,bits_(0)
,bitsLeft_(0)
{
  this->bits_ = this->reader_.readU8();
  this->bitsLeft_  = 8;
}

std::shared_ptr<Parser::Result> Parser::parse() {
  if (this->result_) {
    return this->result_;
  }
  std::vector<Parser::Result::Packet> packets;
  while(!this->reader_.consumed()) {
    std::optional<Parser::Result::Packet> packet = this->parsePacket();
    if (packet.has_value()) {
      packets.emplace_back(std::move(packet.value()));
    }
  }
  this->result_ = std::make_shared<Result>(std::move(this->buffer_), std::move(packets));
  return this->result_;
}

std::optional<Parser::Result::Packet> Parser::parsePacket() {
  size_t const beg = posInBytes();
  Header hdr = parseHeader();
  uint32_t size = 0;
  if(hdr.hasSizeField) {
    size = readLEB128();
  } else {
    size = this->buffer_.size() - 1 - (hdr.extensionFlag ? 1 : 0);
  }
  size_t const startPositionInBytes = posInBytes();
  size_t const end = startPositionInBytes + size;
  size_t const startPosition = this->posInBits();
  if(
      hdr.type != Header::Type::SequenceHeader &&
      hdr.type != Header::Type::TemporalDelimiter &&
      this->OperatingPointIdc != 0 &&
      hdr.extensionFlag
      ) {
    ExtensionHeader ehdr = hdr.extensionHeader.value();
    bool inTemporalLayer = (static_cast<uint16_t>(OperatingPointIdc >> ehdr.temporalID ) & 1u) == 1u;
    bool inSpatialLayer = (static_cast<uint16_t>(OperatingPointIdc >> (ehdr.spatialID + 8u)) & 1u) == 1u;
    if( !inTemporalLayer || !inSpatialLayer) {
      this->reader_.seek(end);
      return std::optional<Parser::Result::Packet>();
    }
  }
  Result::Packet::Content content;
  switch(hdr.type) {
    case Header::Type::Reserved:
      break;
    case Header::Type::SequenceHeader:
      content = this->parseSequenceHeader();
      break;
    case Header::Type::TemporalDelimiter:
      // 5.6.
      // Note: The temporal delimiter has an empty payload.
      if(size != 0) {
        throw std::runtime_error(tfm::format("Invalid temporal delimiter with size=%ld", size));
      }
      content = TemporalDelimiter();
      break;
    case Header::Type::FrameHeader:
    case Header::Type::TileGroup:
    case Header::Type::Metadata:
    case Header::Type::Frame:
    case Header::Type::RedunduntFrameHeader:
    case Header::Type::TileList:
          /* 9-14: reserved */
      break;
    case Header::Type::Padding:
      content = Padding();
      break;
    default:
      throw std::runtime_error(tfm::format("unknown obu type = %d", static_cast<uint8_t>(hdr.type)));
  }
  size_t const currentPosition = this->posInBits();
  size_t const payloadBits = currentPosition - startPosition;
  if (
      size > 0 &&
      hdr.type != Header::Type::TileGroup &&
      hdr.type != Header::Type::TileList &&
      hdr.type != Header::Type::Frame) {
    size_t bitsToRead = size * 8 - payloadBits;
    // 5.3.4. Trailing bits syntax
    uint8_t trailingOneBit = readBits(1);
    if(trailingOneBit != 1u) {
      throw std::runtime_error("trailing_one_bit must be 1, but got 0. Is that a corrupted file?");
    }
    bitsToRead--;
    uint8_t zero;
    while(bitsToRead >= 8u) {
      zero = this->readU8();
      if(zero != 0u) {
        throw std::runtime_error(tfm::format("trailing_zero_bit must be 0, but got %d. Is that a corrupted file?", zero));
      }
      bitsToRead -= 8u;
    }
    zero = this->readBits(bitsToRead);
    if(zero != 0u) {
      throw std::runtime_error(tfm::format("trailing_zero_bit must be 0, but got %d. Is that a corrupted file?", zero));
    }
  }
  this->reader_.seek(end);
  return Result::Packet(beg, end, hdr, std::move(content));
}

Header Parser::parseHeader() {
  Header hdr{};
  hdr.forbidden = readBool();
  hdr.type = static_cast<Header::Type>(readBits(4));
  hdr.extensionFlag = readBool();
  hdr.hasSizeField = readBool();
  hdr.reserved1bit = readBool();
  if(hdr.extensionFlag) {
    ExtensionHeader ehdr{};
    ehdr.temporalID = readBits(3);
    ehdr.spatialID = readBits(2);
    ehdr.spatialID = readBits(3);
    hdr.extensionHeader = ehdr;
  }
  return hdr;
}

SequenceHeader Parser::parseSequenceHeader() {
  SequenceHeader shdr{};
  shdr.seqProfile = this->readBits(3);
  shdr.stillPicture = this->readBool();
  shdr.reducedStillPictureHeader = this->readBool();
  if (shdr.reducedStillPictureHeader) {
    shdr.timingInfoPresentFlag = false;
    shdr.decoderModelInfoPresentFlag = false;
    shdr.initialDisplayDelayPresentFlag = false;
    shdr.operatingPointsCnt = 1;
    shdr.operatingPoints.resize(shdr.operatingPointsCnt);
    shdr.operatingPoints[0].idc = 0;
    shdr.operatingPoints[0].seqLevelIdx = 0;
    shdr.operatingPoints[0].seqTier = 0;
    shdr.operatingPoints[0].decoderModelPresentFlag = false;
    shdr.operatingPoints[0].initialDisplayDelayPresent = false;
  } else {
    shdr.timingInfoPresentFlag = readBool();
    if (shdr.timingInfoPresentFlag) {
      shdr.timingInfo = this->parseTimingInfo();
      shdr.decoderModelInfoPresentFlag = this->readBool();
      if (shdr.decoderModelInfoPresentFlag) {
        shdr.decoderModelInfo = this->parseDecoderModelInfo();
      }
    } else {
      shdr.decoderModelInfoPresentFlag = false;
    }
    shdr.initialDisplayDelayPresentFlag = readBool();
    shdr.operatingPointsCnt = readBits(5) + 1;
    shdr.operatingPoints.resize(shdr.operatingPointsCnt);
    for (size_t i = 0; i < shdr.operatingPointsCnt; ++i) {
      SequenceHeader::OperatingPoint& pt = shdr.operatingPoints[i];
      pt.idc = readUint(12);
      pt.seqLevelIdx = readBits(5);
      if (pt.seqLevelIdx > 7) {
        pt.seqTier = readBits(1);
      } else {
        pt.seqTier = 0;
      }
      if (shdr.decoderModelInfoPresentFlag) {
        SequenceHeader::DecoderModelInfo& decoderModelInfo = shdr.decoderModelInfo.value();
        pt.decoderModelPresentFlag = readBool();
        if (pt.decoderModelPresentFlag) {
          SequenceHeader::OperatingPoint::OperatingParametersInfo info{};
          info.decoderBufferDelay = readUint(decoderModelInfo.bufferDelayLength);
          info.encoderBufferDelay = readUint(decoderModelInfo.bufferDelayLength);
          info.lowDelayModeFlag = readBool();
          pt.operatingParametersInfo = info;
        }
      } else {
        pt.decoderModelPresentFlag = false;
      }
      if (shdr.initialDisplayDelayPresentFlag) {
        pt.initialDisplayDelayPresent = readBool();
        if (pt.initialDisplayDelayPresent) {
          pt.initialDisplayDelay = readBits(4) + 1;
        }
      }
    }
  }
  shdr.frameWidthBits = readBits(4) + 1;
  shdr.frameHeightBits = readBits(4) + 1;
  shdr.maxFrameWidth = readBits(shdr.frameWidthBits) + 1;
  shdr.maxFrameHeight = readBits(shdr.frameHeightBits) + 1;
  if (shdr.reducedStillPictureHeader) {
    shdr.frameIDNumbersPresentFlag = false;
  } else {
    shdr.frameIDNumbersPresentFlag = readBool();
  }
  if (shdr.frameIDNumbersPresentFlag) {
    shdr.deltaFrameIDLength = readBits(4) + 2;
    shdr.additionalFrameIDLength = readBits(3) + 1;
  }
  shdr.use128x128Superblock = readBool();
  shdr.enableFilterIntra = readBool();
  shdr.enableIntraEdgeFilter = readBool();
  if (shdr.reducedStillPictureHeader) {
    shdr.enableInterintraCompound = false;
    shdr.enableMaskedCompound = false;
    shdr.enableWarpedMotion = false;
    shdr.enableDualFilter = false;
    shdr.enableOrderHint = false;
    shdr.enableJNTComp = false;
    shdr.enableRefFrameMVS = false;
    shdr.seqForceScreenContentTools = SequenceHeader::SELECT_SCREEN_CONTENT_TOOLS;
    shdr.seqForceIntegerMV = SequenceHeader::SELECT_INTEGER_MV;
  } else {
    shdr.enableInterintraCompound = readBool();
    shdr.enableMaskedCompound = readBool();
    shdr.enableWarpedMotion = readBool();
    shdr.enableDualFilter = readBool();
    shdr.enableOrderHint = readBool();
    if(shdr.enableOrderHint) {
      shdr.enableJNTComp = readBool();
      shdr.enableRefFrameMVS = readBool();
    } else {
      shdr.enableJNTComp = false;
      shdr.enableRefFrameMVS = false;
    }
    shdr.seqChooseScreenContentTools = readBool();
    if (shdr.seqChooseScreenContentTools) {
      shdr.seqForceScreenContentTools = SequenceHeader::SELECT_SCREEN_CONTENT_TOOLS;
    } else {
      shdr.seqForceScreenContentTools = readBits(1);
    }
    if (shdr.seqChooseScreenContentTools > 0) {
      shdr.seqChooseIntegerMV = readBool();
      if (shdr.seqChooseIntegerMV) {
        shdr.seqForceIntegerMV = SequenceHeader::SELECT_INTEGER_MV;
      } else {
        shdr.seqForceIntegerMV = readBool();
      }
    } else {
      shdr.seqForceIntegerMV = SequenceHeader::SELECT_INTEGER_MV;
    }
    if (shdr.enableOrderHint) {
      shdr.orderHintBits = readBits(3) + 1;
    } else {
      shdr.orderHintBits = 0;
    }
  }
  shdr.enableSuperres = readBool();
  shdr.enableCDEF = readBool();
  shdr.enableRestoration = readBool();
  shdr.colorConfig = this->parseColorConfig(shdr);
  shdr.filmGrainParamsPresent = readBool();
  return shdr;
}

SequenceHeader::TimingInfo Parser::parseTimingInfo() {
  SequenceHeader::TimingInfo info{};
  info.numUnitsInDelayTick = readU32();
  info.timeScale = readU32();
  info.equalPictureInterval = readBool();
  info.numTicksPerPicture = readUVLC();
  return info;
}

SequenceHeader::DecoderModelInfo Parser::parseDecoderModelInfo() {
  SequenceHeader::DecoderModelInfo info{};
  info.bufferDelayLength = readBits(5) + 1;
  info.numUnitsInDecodingTick = readU32();
  info.bufferRemovalTimeLength = readBits(5) + 1;
  info.framePresentationTimeLength = readBits(5) + 1;
  return info;
}

SequenceHeader::ColorConfig Parser::parseColorConfig(SequenceHeader const& shdr) {
  SequenceHeader::ColorConfig cfg{};
  uint8_t bitDepth = 0;
  cfg.highBitdepth = readBits(1);
  if (shdr.seqProfile == 2 && cfg.highBitdepth) {
    cfg.twelveBit = readBool();
    bitDepth = cfg.twelveBit ? 12 : 10;
  } else if (shdr.seqProfile <= 2) {
    bitDepth = cfg.highBitdepth ? 10 : 8;
  } else {
    throw std::runtime_error(tfm::format("unknown or unsupported seq profile: %d", shdr.seqProfile));
  }
  if (shdr.seqProfile == 1) {
    cfg.monochrome = false;
  } else {
    cfg.monochrome = readBool();
  }
  cfg.colorDescriptionPresentFlag = readBool();
  if (cfg.colorDescriptionPresentFlag) {
    cfg.colorPrimaries = static_cast<SequenceHeader::ColorConfig::ColorPrimaries>(readU8());
    cfg.transferCharacteristics = static_cast<SequenceHeader::ColorConfig::TransferCharacteristics>(readU8());
    cfg.matrixCoefficients = static_cast<SequenceHeader::ColorConfig::MatrixCoefficients>(readU8());
  }
  if (cfg.monochrome) {
    cfg.colorRange = readBool();
    cfg.subsamplingX = 1;
    cfg.subsamplingY = 1;
    cfg.chromaSamplePosition = std::optional<uint8_t>();
    cfg.separateUVDeltaQ = false;
    return cfg;
  } else if (
      cfg.colorPrimaries == SequenceHeader::ColorConfig::ColorPrimaries::CP_BT_709 &&
          cfg.transferCharacteristics == SequenceHeader::ColorConfig::TransferCharacteristics::TC_SRGB &&
          cfg.matrixCoefficients == SequenceHeader::ColorConfig::MatrixCoefficients::MC_IDENTITY) {
    cfg.colorRange = true;
    cfg.subsamplingX = 0;
    cfg.subsamplingY = 0;
  } else {
    cfg.colorRange = readBool();
    if (shdr.seqProfile == 0) {
      cfg.subsamplingX = 1;
      cfg.subsamplingY = 1;
    } else if (shdr.seqProfile == 1) {
      cfg.subsamplingX = 0;
      cfg.subsamplingY = 0;
    } else {
      if(bitDepth == 12) {
        cfg.subsamplingX = readBits(1);
        if (cfg.subsamplingX != 0) {
          cfg.subsamplingY = readBits(1);
        } else {
          cfg.subsamplingY = 0;
        }
      } else {
        cfg.subsamplingX = 1;
        cfg.subsamplingY = 0;
      }
    }
    if (cfg.subsamplingX != 0 && cfg.subsamplingY != 0) {
      cfg.chromaSamplePosition = readBits(2);
    }
  }
  cfg.separateUVDeltaQ = readBool();
  return cfg;
}

uint32_t Parser::readLEB128() {
  uint64_t value = 0;
  for(size_t i = 0; i < 8; i++) {
    uint8_t const v = readU8();
    value |= static_cast<uint64_t>(v & 0x7fu) << (i*7u);
    if ( (v & 0x80u) != 0x80u ) {
      break;
    }
  }
  return value;
}

uint32_t Parser::readUVLC() {
  uint32_t leadingZeros = 0;
  while(true) {
    bool done = readBool();
    if(done) {
      break;
    }
    leadingZeros++;
  }
  if(leadingZeros >= 32) {
    return 0xffffffff;
  }
  uint32_t value = 0;
  uint32_t left = 0;
  while(left < 8) {
    value = value << 8u | readU8();
    left -= 8;
  }
  value = value << left | readBits(left);
  return value + (1u << leadingZeros) - 1u;
}

uint8_t Parser::readBits(uint8_t const bits) {
  assert(bits <= 8 && "readBits can read less then or equal to 8 bits.");
  if(bits <= this->bitsLeft_) {
    auto const result = static_cast<uint8_t>(bits_ >> static_cast<uint8_t>(this->bitsLeft_ - bits)) & ((1u << bits)-1u);
    this->bitsLeft_ -= bits;
    return result;
  }
  uint8_t v = this->reader_.readU8();
  uint8_t nextBitsLeft = bitsLeft_ + 8u - bits;
  uint8_t upper = static_cast<uint8_t>(bits_ & ((1u << bitsLeft_) - 1u)) << static_cast<uint8_t>(bits - bitsLeft_);
  uint8_t lower = static_cast<uint8_t>(v >> nextBitsLeft) & ((1u << static_cast<uint8_t>(bits - bitsLeft_)) - 1);
  auto const result = upper | lower;
  this->bitsLeft_ = nextBitsLeft;
  this->bits_ = v;
  return result;
}

bool Parser::readBool() {
  return this->readBits(1) == 1u;
}

uint8_t Parser::readU8() {
  return readBits(8);
}

uint16_t Parser::readU16() {
  return
      static_cast<uint16_t>(static_cast<uint16_t>(readU8()) << 8u) |
      static_cast<uint16_t>(static_cast<uint16_t>(readU8()) << 0u);
}

uint32_t Parser::readU32() {
  return
      static_cast<uint32_t>(readU8()) << 24u |
      static_cast<uint32_t>(readU8()) << 16u |
      static_cast<uint32_t>(readU8()) << 8u |
      static_cast<uint32_t>(readU8()) << 0u;
}

uint64_t Parser::readU64() {
  return
      static_cast<uint64_t>(readU8()) << 56u |
      static_cast<uint64_t>(readU8()) << 48u |
      static_cast<uint64_t>(readU8()) << 40u |
      static_cast<uint64_t>(readU8()) << 32u |
      static_cast<uint64_t>(readU8()) << 24u |
      static_cast<uint64_t>(readU8()) << 16u |
      static_cast<uint64_t>(readU8()) << 8u |
      static_cast<uint64_t>(readU8()) << 0u;
}

uint64_t Parser::readUint(size_t const bits) {
  assert(bits <= 64 && "readUint can read less then or equal to 64 bits.");
  size_t left = bits;
  uint64_t value = 0u;
  while(left >= 8) {
    value = value << 8u | readU8();
    left -= 8u;
  }
  value = value << left | readBits(left);
  return value;
}

}