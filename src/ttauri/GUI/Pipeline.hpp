// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "../required.hpp"
#include <memory>

namespace tt {
class gui_window;

class pipeline {
public:
    gui_window const &window;

    pipeline(gui_window const &window);

    virtual ~pipeline() = default;
    pipeline(const pipeline &) = delete;
    pipeline &operator=(const pipeline &) = delete;
    pipeline(pipeline &&) = delete;
    pipeline &operator=(pipeline &&) = delete;
};

}
