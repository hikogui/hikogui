// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "GridLayoutWidget.hpp"
#include "../GUI/Theme.hpp"
#include <memory>

namespace tt {

class ColumnWidget final : public GridLayoutWidget {
public:
    ColumnWidget(Window &window, Widget *parent) noexcept :
        GridLayoutWidget(window, parent) {}

    /** Add a widget directly to this widget.
    *
    * Thread safety: modifies atomic. calls addWidget() and addWidgetDirectly()
    */
    template<typename T, typename... Args>
    T &makeWidget(Args &&... args) {
        return GridLayoutWidget::makeWidget<T,"L0T+1"_ca>(std::forward<Args>(args)...);
    }
};

}
