// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

namespace tt {
class ContainerWidget;

class ContainerWidgetDelegate {
public:
    virtual void openingContainerWidget(ContainerWidget &widget) noexcept {}
    virtual void closingContainerWidget(ContainerWidget &widget) noexcept {}
};

}