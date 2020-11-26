// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "../required.hpp"
#include <type_traits>
#include <memory>

namespace tt {
class grid_layout_widget;

class grid_layout_delegate {
public:
    virtual void init(grid_layout_widget &self) noexcept {}
    virtual void deinit(grid_layout_widget &self) noexcept {}
};

}
