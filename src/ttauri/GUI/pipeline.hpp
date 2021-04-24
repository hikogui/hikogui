// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../required.hpp"
#include <memory>

namespace tt {
class gui_surface;

class pipeline {
public:
    gui_surface const &surface;

    pipeline(gui_surface const &surface);

    virtual ~pipeline() = default;
    pipeline(const pipeline &) = delete;
    pipeline &operator=(const pipeline &) = delete;
    pipeline(pipeline &&) = delete;
    pipeline &operator=(pipeline &&) = delete;
};

}
