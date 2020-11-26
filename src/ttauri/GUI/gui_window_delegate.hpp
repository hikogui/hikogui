// Copyright 2019, 2020 Pokitec
// All rights reserved.

#pragma once

#include "../widgets/grid_layout_delegate.hpp"

namespace tt {
class gui_window;

class gui_window_delegate : public grid_layout_delegate {
public:
    virtual void init(gui_window &window) noexcept {}
    virtual void deinit(gui_window &window) noexcept {}
};

}
