// Copyright 2019, 2020 Pokitec
// All rights reserved.

#pragma once

#include "../widgets/GridLayoutDelegate.hpp"

namespace tt {
class gui_window;

class WindowDelegate : public GridLayoutDelegate {
public:
    virtual void openingWindow(gui_window &window) noexcept {}
    virtual void closingWindow(gui_window &window) noexcept {}
};

}
