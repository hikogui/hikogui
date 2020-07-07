// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "ttauri/widgets/Widget.hpp"

namespace tt {

class ContainerWidget : public Widget {
public:
    ContainerWidget(Window &window, Widget *parent) noexcept:
        Widget(window, parent, vec{0.0, 0.0}) {}

    ~ContainerWidget() {}

    ContainerWidget(const ContainerWidget &) = delete;
    ContainerWidget &operator=(const ContainerWidget &) = delete;
    ContainerWidget(ContainerWidget &&) = delete;
    ContainerWidget &operator=(ContainerWidget &&) = delete;
};

}