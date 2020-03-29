//
// Created by psi on 2020/01/08.
//

#include <cassert>
#include "BitStreamReader.hpp"

namespace avif::av1 {

BitStreamReader::BitStreamReader(avif::util::Logger& log, std::vector<uint8_t> const& buffer)
:reader_(log, buffer)
,log_(log)
,bits_(0)
,posInBits_(0)
{
  this->bits_ = this->reader_.readU8();
}

size_t BitStreamReader::posInBits() const {
  return (posInBytes() * 8u) + posInBits_;
}

size_t BitStreamReader::posInBytes() const {
  return posInBits_ == 8 ? reader_.pos() : (reader_.pos() - 1u);
}

bool BitStreamReader::consumed() const {
  return reader_.consumed() && posInBits_ == 8;
}

void BitStreamReader::seekInBytes(size_t posInBytes) {
  this->reader_.seek(posInBytes);
  if (!reader_.consumed()) {
    this->bits_ = this->reader_.readU8();
    this->posInBits_ = 0;
  } else {
    this->bits_ = 0;
    this->posInBits_ = 8;
  }
}

uint32_t BitStreamReader::readLEB128() {
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

uint32_t BitStreamReader::readUVLC() {
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

uint8_t BitStreamReader::readBits(uint8_t const bits) {
  assert(bits <= 8 && "readBits can read less then or equal to 8 bits.");
  uint8_t const nextPosInBits = posInBits_ + bits;
  if(this->reader_.consumed() && nextPosInBits > 8) {
    throw std::range_error("Buffer overrun.");
  }
  if (nextPosInBits < 8 || (nextPosInBits == 8 && this->reader_.consumed())) {
    posInBits_ = nextPosInBits;
    return static_cast<uint8_t>(bits_ >> (8u - posInBits_)) & ((1u << bits)-1u);
  } else {
    uint8_t const next = this->reader_.readU8();
    uint8_t upper = static_cast<uint8_t>(bits_ & ((1u << (8u - posInBits_)) - 1u)) << (posInBits_ + bits - 8u);
    uint8_t lower = static_cast<uint8_t>(next >> (16u - nextPosInBits)) & ((1u << (posInBits_ + bits - 8u)) - 1u);
    posInBits_ = nextPosInBits - 8;
    bits_ = next;
    return upper | lower;
  }
}

bool BitStreamReader::readBool() {
  return this->readBits(1) == 1u;
}

uint8_t BitStreamReader::readU8() {
  return readBits(8);
}

uint16_t BitStreamReader::readU16() {
  return
      static_cast<uint16_t>(static_cast<uint16_t>(readU8()) << 8u) |
      static_cast<uint16_t>(static_cast<uint16_t>(readU8()) << 0u);
}

uint32_t BitStreamReader::readU32() {
  return
      static_cast<uint32_t>(readU8()) << 24u |
      static_cast<uint32_t>(readU8()) << 16u |
      static_cast<uint32_t>(readU8()) << 8u |
      static_cast<uint32_t>(readU8()) << 0u;
}

uint64_t BitStreamReader::readU64() {
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

uint64_t BitStreamReader::readUint(size_t const bits) {
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