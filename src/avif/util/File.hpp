//
// Created by psi on 2020/01/12.
//

#pragma once

#pragma once
#include <variant>
#include <optional>
#include <vector>
#include <string>

namespace avif::util {

std::variant<std::vector<uint8_t>, std::string> readFile(std::string const& fname);
std::optional<std::string> writeFile(std::string const& fname, std::vector<uint8_t> const& data);

}
