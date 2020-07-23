// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "ContainerWidget.hpp"
#include "../GUI/Theme.hpp"
#include <memory>

namespace tt {

class RowWidget : public ContainerWidget {
protected:
    rhea::constraint rightConstraint;

public:
    RowWidget(Window &window, Widget *parent) noexcept :
        ContainerWidget(window, parent) {}

    Widget &addWidget(cell_address address, std::unique_ptr<Widget> childWidget) noexcept override;

    /** Add a widget directly to this widget.
    *
    * Thread safety: modifies atomic. calls addWidget() and addWidgetDirectly()
    */
    template<typename T, typename... Args>
    T &makeWidget(Args &&... args) {
        return ContainerWidget::makeWidget<T,"L+1"_ca>(std::forward<Args>(args)...);
    }};

}
