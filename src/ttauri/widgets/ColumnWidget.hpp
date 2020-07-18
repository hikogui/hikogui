// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "Widget.hpp"
#include "../GUI/Theme.hpp"
#include <memory>

namespace tt {

class ColumnWidget : public Widget {
protected:
    rhea::constraint bottomConstraint;

public:
    ColumnWidget(Window &window, Widget *parent) noexcept :
        Widget(window, parent, 0.0f, 0.0f) {}

    Widget &addWidget(Alignment alignment, std::unique_ptr<Widget> childWidget) noexcept override;
};

}
