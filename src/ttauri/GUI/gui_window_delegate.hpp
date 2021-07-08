// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../widgets/grid_delegate.hpp"

namespace tt {
class gui_window;

class gui_window_delegate : public grid_delegate {
public:
    virtual void init(gui_window &window) noexcept {}
    virtual void deinit(gui_window &window) noexcept {}
};

}
