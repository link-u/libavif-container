//
// Created by psi on 2019/11/24.
//

#pragma once

#include <string>
#include <iomanip>
#include <sstream>
#include <fmt/format.h>

namespace avif::util {

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

protected:
  explicit Logger(Level level)
      :level_(level)
  {
  }

public:
  Logger() = delete;
  Logger(Logger const&) = delete;
  Logger(Logger&&) = delete;
  Logger& operator=(Logger const&) = delete;
  Logger& operator=(Logger&&) = delete;
  virtual ~Logger() noexcept = default;

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
  [[ noreturn ]] void fatal(std::string const &fmt, Args &&...args) {
    std::string msg = this->log_(Level::FATAL, fmt, std::forward<Args>(args)...);
    throw std::runtime_error(msg);
  }

  void setLevel(const Level level) {
    this->level_ = level;
  }

private:
  template<typename ...Args>
  std::string log_(Level const level, std::string const &fmt, Args &&...args) {
    if (level_ > level) {
      return "";
    }
    std::string const msg = fmt::format(fmt.c_str(), std::forward<Args>(args)...);
    time_t t = time(nullptr);
    tm tm = *localtime(&t);
    std::stringstream ss;
    ss << std::put_time(&tm, "%Y/%m/%d %H:%M:%S");
    std::string const time = ss.str();
    std::string output{};
    switch (level) {
      case Level::TRACE:
        output = fmt::format("[{} TRACE] {}", time, msg);
        break;
      case Level::DEBUG:
        output = fmt::format("[{} DEBUG] {}", time, msg);
        break;
      case Level::INFO:
        output = fmt::format("[{} INFO ] {}", time, msg);
        break;
      case Level::WARN:
        output = fmt::format("[{} WARN ] {}", time, msg);
        break;
      case Level::ERROR:
        output = fmt::format("[{} ERROR] {}", time, msg);
        break;
      case Level::FATAL:
        output = fmt::format("[{} FATAL] {}", time, msg);
    }
    this->writeLog_(level, output);
    return msg;
  }
  virtual void writeLog_(Level lv, std::string const& msg) = 0;
};

}


