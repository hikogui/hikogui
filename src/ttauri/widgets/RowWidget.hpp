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

    Widget &addWidget(WidgetPosition position, std::unique_ptr<Widget> childWidget) noexcept override;
};

}
