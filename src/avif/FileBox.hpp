//
// Created by psi on 2019/11/24.
//

#pragma once

#include "MetaBox.hpp"
#include "FileTypeBox.hpp"
#include "MediaDataBox.hpp"

namespace avif {

struct FileBox : public Box {
  FileTypeBox fileTypeBox;
  MetaBox metaBox;
  MediaDataBox mediaDataBox;
};

}