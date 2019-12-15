//
// Created by psi on 2019/11/24.
//

#pragma once

#include <string>
#include <iomanip>

#include "../../external/tinyformat/tinyformat.h"

namespace util {

class Logger {
public:
  enum Level : int {
    TRACE = 0,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL,
  };
private:
  Level level_;
  FILE *const output_;
  FILE *const errorOutput_;
public:
  Logger() = delete;
  Logger(Logger const&) = delete;
  Logger(Logger&&) = delete;
  Logger& operator=(Logger const&) = delete;
  Logger& operator=(Logger&&) = delete;

public:
  Logger(FILE* output, FILE* errOutput, Level level)
  :output_(output)
  ,errorOutput_(errOutput)
  ,level_(level)
  {
  }

public:
  template<typename ...Args>
  void trace(std::string const &fmt, Args &&...args) {
    this->log_(Level::TRACE, fmt, std::forward<Args>(args)...);
  }

  template<typename ...Args>
  void debug(std::string const &fmt, Args &&...args) {
    this->log_(Level::DEBUG, fmt, std::forward<Args>(args)...);
  }

  template<typename ...Args>
  void info(std::string const &fmt, Args &&...args) {
    this->log_(Level::INFO, fmt, std::forward<Args>(args)...);
  }

  template<typename ...Args>
  void warn(std::string const &fmt, Args &&...args) {
    this->log_(Level::WARN, fmt, std::forward<Args>(args)...);
  }

  template<typename ...Args>
  void error(std::string const &fmt, Args &&...args) {
    this->log_(Level::ERROR, fmt, std::forward<Args>(args)...);
  }

  template<typename ...Args>
  void fatal(std::string const &fmt, Args &&...args) {
    this->log_(Level::FATAL, fmt, std::forward<Args>(args)...);
  }

  void setLevel(const Level level) {
    this->level_ = level;
  }

private:
  template<typename ...Args>
  void log_(Level const level, std::string const &fmt, Args &&...args) {
    if (level_ > level) {
      return;
    }
    std::string const msg = tfm::format(fmt.c_str(), std::forward<Args>(args)...);
    time_t t = time(nullptr);
    tm tm = *localtime(&t);
    std::stringstream ss;
    ss << std::put_time(&tm, "%Y/%m/%d %H:%M:%S");
    std::string const time = ss.str();
    switch (level) {
      case Level::TRACE:
        fprintf(this->output_, "[%s TRACE] %s\n", time.c_str(), msg.c_str());
        break;
      case Level::DEBUG:
        fprintf(this->output_, "[%s DEBUG] %s\n", time.c_str(), msg.c_str());
        break;
      case Level::INFO:
        fprintf(this->output_, "[%s INFO ] %s\n", time.c_str(), msg.c_str());
        break;
      case Level::WARN:
        fprintf(this->output_, "[%s WARN ] %s\n", time.c_str(), msg.c_str());
        break;
      case Level::ERROR:
        fprintf(this->errorOutput_, "[%s ERROR] %s\n", time.c_str(), msg.c_str());
        break;
      case Level::FATAL:
        std::string err = tfm::format("[%s FATAL] %s\n", time, msg);
        fputs(err.c_str(), this->errorOutput_);
        throw std::runtime_error(err);
    }
    fflush(this->output_);
  }
};

}


