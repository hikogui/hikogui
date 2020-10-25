// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "../required.hpp"
#include <type_traits>
#include <memory>

namespace tt {
class GridLayoutWidget;

class GridLayoutDelegate {
public:
    virtual void openingWidget(std::shared_ptr<GridLayoutWidget> widget) noexcept {}
    virtual void closingWidget() noexcept {}
};

}
