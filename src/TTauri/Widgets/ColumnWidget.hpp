// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Widgets/Widget.hpp"
#include "TTauri/GUI/Theme.hpp"
#include <memory>

namespace tt {

class ColumnWidget : public Widget {
protected:
    rhea::constraint bottomConstraint;

public:
    ColumnWidget(Window &window, Widget *parent) noexcept :
        Widget(window, parent, vec{0.0f, 0.0f}) {}

    Widget &addWidget(Alignment alignment, std::unique_ptr<Widget> childWidget) noexcept override
    {
        auto *previous_widget = ssize(children) != 0 ? children.back().get() : nullptr;
        if (previous_widget) {
            window.removeConstraint(bottomConstraint);
        }

        auto &widget = Widget::addWidget(alignment, std::move(childWidget));
        if (previous_widget) {
            widget.placeBelow(*previous_widget);
        } else {
            widget.placeAtTop(0.0f);
        }
        widget.placeLeft(0.0f);
        widget.placeRight(0.0f);
        bottomConstraint = widget.placeAtBottom(0.0f);

        return widget;
    }
};

}
