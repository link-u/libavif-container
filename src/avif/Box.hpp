//
// Created by psi on 2019/11/24.
//

#pragma once

#include <optional>
#include <string>

namespace avif {

struct Box {
protected:
  Box() = default;
  Box(Box&&) = default;
  Box(Box const&) = default;
  Box& operator=(Box&&) = default;
  Box& operator=(Box const&) = default;
};

}



