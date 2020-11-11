// Copyright 2019, 2020 Pokitec
// All rights reserved.

#pragma once

#include "../widgets/GridLayoutDelegate.hpp"

namespace tt {
class Window_base;

class WindowDelegate : public GridLayoutDelegate {
public:
    virtual void openingWindow(Window_base &window) noexcept {}
    virtual void closingWindow(Window_base &window) noexcept {}
};

}
