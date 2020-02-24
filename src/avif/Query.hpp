//
// Created by psi on 2020/02/25.
//

#pragma once

#include <optional>
#include "FileBox.hpp"

namespace avif::util::query {

template <typename T>
std::optional<T> findProperty(avif::FileBox const& fileBox, std::optional<uint32_t> itemID) {
  for(auto const& assoc : fileBox.metaBox.itemPropertiesBox.associations){
    for(auto const& item : assoc.items) {
      if(itemID.has_value() && item.itemID != itemID.value()) {
        continue;
      }
      for(auto const& entry : item.entries) {
        if(entry.propertyIndex == 0) {
          continue;
        }
        auto& prop = fileBox.metaBox.itemPropertiesBox.propertyContainers.properties.at(entry.propertyIndex - 1);
        if(std::holds_alternative<T>(prop)){
          return std::get<T>(prop);
        }
      }
    }
  }
  return std::optional<T>();
}

std::pair<size_t, size_t> findItemRegion(avif::FileBox const& fileBox, std::optional<uint32_t> const itemID, std::optional<uint32_t> const extentID = {}) {
  size_t const itemIdx = itemID.has_value() ? (itemID.value() - 1) : 0;
  size_t const extentIdx = extentID.has_value() ? (extentID.value() - 1) : 0;
  size_t const baseOffset = fileBox.metaBox.itemLocationBox.items.at(itemIdx).baseOffset;
  size_t const extentOffset = fileBox.metaBox.itemLocationBox.items.at(itemIdx).extents.at(extentIdx).extentOffset;
  size_t const extentLength = fileBox.metaBox.itemLocationBox.items.at(itemIdx).extents.at(extentIdx).extentLength;
  return std::make_pair(baseOffset + extentOffset, baseOffset + extentOffset + extentLength);
}

std::optional<uint32_t> findPrimaryItemID(avif::FileBox const& fileBox) {
  if(fileBox.metaBox.primaryItemBox.has_value()) {
    return fileBox.metaBox.primaryItemBox.value().itemID;
  }
  return std::optional<uint32_t>();
}

std::optional<uint32_t> findAuxItemID(avif::FileBox const& fileBox, uint32_t const itemID, std::string const& auxType) {
  if(!fileBox.metaBox.itemReferenceBox.has_value()) {
    return std::optional<uint32_t>();
  }
  auto const& refs = fileBox.metaBox.itemReferenceBox->references;
  if(std::holds_alternative<std::vector<SingleItemTypeReferenceBoxLarge>>(refs)) {
    for(auto const& ref : std::get<std::vector<SingleItemTypeReferenceBoxLarge>>(refs)) {
      for(auto const& toItemID : ref.toItemIDs) {
        if(toItemID == itemID) {
          for(auto const& assoc : fileBox.metaBox.itemPropertiesBox.associations){
            for(auto const& item : assoc.items){
              if(item.itemID == ref.fromItemID) {
                for(auto const& prop : fileBox.metaBox.itemPropertiesBox.propertyContainers.properties) {
                  if(std::holds_alternative<AuxiliaryTypeProperty>(prop)) {
                    if(std::get<AuxiliaryTypeProperty>(prop).auxType == auxType) {
                      return ref.fromItemID;
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  } else {
    for(auto const& ref : std::get<std::vector<SingleItemTypeReferenceBox>>(refs)) {
      for(auto const& toItemID : ref.toItemIDs) {
        if(toItemID == itemID) {
          for(auto const& assoc : fileBox.metaBox.itemPropertiesBox.associations){
            for(auto const& item : assoc.items){
              if(item.itemID == ref.fromItemID) {
                for(auto const& prop : fileBox.metaBox.itemPropertiesBox.propertyContainers.properties) {
                  if(std::holds_alternative<AuxiliaryTypeProperty>(prop)) {
                    if(std::get<AuxiliaryTypeProperty>(prop).auxType == auxType) {
                      return ref.fromItemID;
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  return std::optional<uint32_t>();
}

}