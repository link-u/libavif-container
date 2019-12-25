//
// Created by psi on 2019/11/24.
//

#pragma once

#include "Box.hpp"
#include "FullBox.hpp"
#include "HandlerBox.hpp"
#include "ItemLocationBox.hpp"
#include "ItemPropertiesBox.hpp"
#include "ItemInfoBox.hpp"

namespace avif {

struct MetaBox : public FullBox {
  HandlerBox handlerBox;
  ItemPropertiesBox itemPropertiesBox;
  ItemLocationBox itemLocationBox;
  ItemInfoBox itemInfoBox;
};

}
