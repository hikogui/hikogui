// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "../required.hpp"
#include <type_traits>

namespace tt {
class GridWidget;

class GridWidgetDelegate {
public:
    virtual void openingWidget(GridWidget &widget) noexcept {};
    virtual void closingWidget(GridWidget &widget) noexcept {};
};

}