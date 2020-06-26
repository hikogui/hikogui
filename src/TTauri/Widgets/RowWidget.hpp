// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Widgets/Widget.hpp"
#include "TTauri/GUI/Theme.hpp"
#include <memory>

namespace tt {

class RowWidget : public Widget {
protected:
    rhea::constraint rightConstraint;

public:
    RowWidget(Window &window, Widget *parent) noexcept :
        Widget(window, parent, vec{0.0f, 0.0f}) {}

    Widget &addWidget(Alignment alignment, std::unique_ptr<Widget> childWidget) noexcept override
    {
        auto *previous_widget = ssize(children) != 0 ? children.back().get() : nullptr;
        if (previous_widget) {
            window.removeConstraint(rightConstraint);
        }

        auto &widget = Widget::addWidget(alignment, std::move(childWidget));
        if (previous_widget) {
            widget.placeRightOf(*previous_widget);
        } else {
            widget.placeLeft(0.0f);
        }
        widget.placeAtTop(0.0f);
        widget.placeAtBottom(0.0f);
        rightConstraint = widget.placeRight(0.0f);

        return widget;
    }
};

}
