//
// Created by psi on 2019/11/25.
//

#pragma once

#include <vector>

#include "Box.hpp"
#include "ItemPropertyContainer.hpp"
#include "ItemPropertyAssociation.hpp"

namespace avif {

// https://github.com/nokiatech/heif/blob/master/srcs/common/itempropertiesbox.cpp
// iprp
struct ItemPropertiesBox : public Box {
  ItemPropertyContainer itemPropertyContainer;
  std::vector<ItemPropertyAssociation> itemPropertyAssociations;
};

}