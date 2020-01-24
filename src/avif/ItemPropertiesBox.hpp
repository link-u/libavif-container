//
// Created by psi on 2019/11/25.
//

#pragma once

#include <vector>

#include "Box.hpp"
#include "ItemPropertyContainer.hpp"
#include "ItemPropertyAssociation.hpp"

namespace avif {

// ISO/IEC 23008-12:2017(E)
// p.29
// iprp, ipma, ipco
struct ItemPropertiesBox : public Box {
  ItemPropertyContainer propertyContainers;
  std::vector<ItemPropertyAssociation> associations;
};

}