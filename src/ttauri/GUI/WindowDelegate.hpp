// Copyright 2019, 2020 Pokitec
// All rights reserved.

#pragma once

#include "Window_forward.hpp"
#include "../widgets/GridWidgetDelegate.hpp"

namespace tt {

class WindowDelegate : public GridWidgetDelegate {
public:
    virtual void openingWindow(Window &window) noexcept {}
    virtual void closingWindow(Window &window) noexcept {}
};

}
