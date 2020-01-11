//
// Created by psi on 2020/01/11.
//

#include <vector>
#include <memory>
#include <gtest/gtest.h>
#include "../../src/avif/av1/Parser.hpp"
#include "../../src/avif/util/FileLogger.hpp"

TEST(AV1Test, parsingSequenceHeaderOBU) {
  using avif::util::FileLogger;
  FileLogger log(stdout, stderr, FileLogger::Level::TRACE);
  static std::vector<uint8_t> const TEST_OBU = {
      {0x0a, 0x0b, 0x20, 0x00, 0x00, 0x42, 0x6b, 0xbf, 0xbc, 0x6f, 0xff, 0xcc, 0x10}
  };
  using avif::av1::Parser;
  using avif::av1::Header;
  using avif::av1::SequenceHeader;

  Parser p = Parser(log, TEST_OBU);
  std::shared_ptr<Parser::Result> result = p.parse();
  ASSERT_TRUE(result->ok());
  ASSERT_EQ(1, result->packets().size());
  Parser::Result::Packet packet = result->packets().at(0);
  ASSERT_EQ(Header::Type::SequenceHeader, packet.type());
  ASSERT_TRUE(std::holds_alternative<SequenceHeader>(packet.content()));
  auto seq = std::get<SequenceHeader>(packet.content());
  ASSERT_TRUE(seq.enableCDEF);
  ASSERT_TRUE(seq.use128x128Superblock);
  ASSERT_FALSE(seq.filmGrainParamsPresent);
  ASSERT_FALSE(seq.enableSuperres);
}