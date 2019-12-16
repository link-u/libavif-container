//
// Created by psi on 2019/12/15.
//

#pragma once

#include "Logger.hpp"

//
// Created by psi on 2019/11/24.
//

#pragma once

#include <string>

#include "Logger.hpp"

namespace avif::util {

class FileLogger final : public Logger {
private:
  FILE* const output_;
  FILE* const errorOutput_;
public:
  FileLogger() = delete;
  FileLogger(FileLogger const&) = delete;
  FileLogger(FileLogger&&) = delete;
  FileLogger& operator=(FileLogger const&) = delete;
  FileLogger& operator=(FileLogger&&) = delete;

public:
  FileLogger(FILE* const output, FILE* const errOutput, Level const level)
      :Logger(level)
      ,output_(output)
      ,errorOutput_(errOutput)
  {
  }

protected:
  void writeLog_(Level lv, std::string const& msg) override;
};

}