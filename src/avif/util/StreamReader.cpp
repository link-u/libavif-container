//
// Created by psi on 2020/01/05.
//

#include "StreamReader.hpp"

namespace avif::util {

uint8_t StreamReader::readU8() {
  uint8_t res = this->buffer_.at(pos_);
  pos_++;
  return res;
}

uint16_t StreamReader::readU16() {
  uint16_t res =
      static_cast<uint16_t>(static_cast<uint16_t>(buffer_.at(pos_)) << 8u) |
      static_cast<uint16_t>(static_cast<uint16_t>(this->buffer_.at(pos_ + 1)) << 0u);
  pos_+=2;
  return res;
}

uint32_t StreamReader::readU32() {
  uint32_t res =
      static_cast<uint32_t>(buffer_.at(pos_ + 0)) << 24u |
      static_cast<uint32_t>(buffer_.at(pos_ + 1)) << 16u |
      static_cast<uint32_t>(buffer_.at(pos_ + 2)) << 8u |
      static_cast<uint32_t>(buffer_.at(pos_ + 3)) << 0u;
  pos_+=4;
  return res;
}

uint64_t StreamReader::readU64() {
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

std::optional<uint64_t> StreamReader::readUint(size_t octets) {
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

std::string StreamReader::readString() {
  size_t const beg = this->pos_;
  size_t end = beg;
  bool found = false;
  for(; end < this->buffer_.size(); ++end) {
    if(this->buffer_.at(end) == '\0') {
      found = true;
      break;
    }
  }
  if(found) {
    this->pos_ = end + 1;
    return std::string(std::next(this->buffer_.begin(), beg), std::next(this->buffer_.begin(), end));
  } else {
    throw std::out_of_range("Filed to read string. File may be corrupted?");
  }
}

}