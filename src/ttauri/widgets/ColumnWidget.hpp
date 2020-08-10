// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "GridWidget.hpp"
#include "../GUI/Theme.hpp"
#include <memory>

namespace tt {

class ColumnWidget : public GridWidget {
public:
    ColumnWidget(Window &window, Widget *parent) noexcept :
        GridWidget(window, parent) {}

    /** Add a widget directly to this widget.
    *
    * Thread safety: modifies atomic. calls addWidget() and addWidgetDirectly()
    */
    template<typename T, typename... Args>
    T &makeWidget(Args &&... args) {
        return GridWidget::makeWidget<T,"L0T+1"_ca>(std::forward<Args>(args)...);
    }

};

}
