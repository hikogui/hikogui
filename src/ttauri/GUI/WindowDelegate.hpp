// Copyright 2019, 2020 Pokitec
// All rights reserved.

#pragma once

#include "Window_forward.hpp"
#include "../widgets/ContainerWidgetDelegate.hpp"

namespace tt {

class WindowDelegate : public ContainerWidgetDelegate {
public:
    virtual void openingWindow(Window &window) noexcept {}
    virtual void closingWindow(Window &window) noexcept {}
};

}
