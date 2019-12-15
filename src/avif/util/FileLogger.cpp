//
// Created by psi on 2019/12/15.
//

#include "FileLogger.hpp"

namespace avif::util {

void FileLogger::writeLog_(Logger::Level lv, std::string const& msg) {
  if(lv < WARN) {
    fprintf(this->output_, "%s\n", msg.c_str());
  } else {
    fprintf(this->errorOutput_, "%s\n", msg.c_str());
  }
}

}

