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

    Widget &addWidget(Alignment alignment, std::unique_ptr<Widget> childWidget) noexcept override;
};

}
