// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "../required.hpp"
#include <type_traits>

namespace tt {
class GridLayoutWidget;

class GridLayoutDelegate {
public:
    virtual void openingWidget(GridLayoutWidget &widget) noexcept {};
    virtual void closingWidget(GridLayoutWidget &widget) noexcept {};
};

}