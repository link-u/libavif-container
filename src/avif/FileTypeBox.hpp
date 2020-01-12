//
// Created by psi on 2019/11/24.
//

#pragma once

#include <string>
#include <vector>

namespace avif{

struct FileTypeBox : public Box {
  std::string majorBrand;
  uint32_t minorVersion;
  std::vector<std::string> compatibleBrands;
};

}