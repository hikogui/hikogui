// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../utility/module.hpp"
#include "../macros.hpp"
#include <memory>

import hikogui_utility;

namespace hi::inline v1 {
class gfx_surface;

class pipeline {
public:
    gfx_surface const &surface;

    pipeline(gfx_surface const &surface) : surface(surface) {}

    virtual ~pipeline() = default;
    pipeline(const pipeline &) = delete;
    pipeline &operator=(const pipeline &) = delete;
    pipeline(pipeline &&) = delete;
    pipeline &operator=(pipeline &&) = delete;
};

} // namespace hi::inline v1
