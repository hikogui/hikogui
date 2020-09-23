// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "../required.hpp"
#include <type_traits>

namespace tt {
class ContainerWidget;
class GridWidget;
class ToolbarWidget;
class ColumnWidget;
class RowWidget;

class ContainerWidgetDelegateBase {
public:
    virtual void openingContainerWidget(ContainerWidget &widget) noexcept = 0;
    virtual void closingContainerWidget(ContainerWidget &widget) noexcept = 0;
};

template<typename T> requires (!std::is_same_v<T,ContainerWidget>)
class ContainerWidgetDelegate: public ContainerWidgetDelegateBase {
public:
    using value_type = T;

    virtual void openingContainerWidget(value_type &widget) noexcept {}
    virtual void closingContainerWidget(value_type &widget) noexcept {}

    virtual void openingContainerWidget(ContainerWidget &widget) noexcept override {
        //tt_assume(dynamic_cast<value_type *>(&widget) != nullptr);
        return openingContainerWidget(*(reinterpret_cast<value_type *>(&widget)));
    }
    virtual void closingContainerWidget(ContainerWidget &widget) noexcept override {
        //tt_assume(dynamic_cast<value_type *>(&widget) != nullptr);
        return closingContainerWidget(*(reinterpret_cast<value_type *>(&widget)));
    }
};

}