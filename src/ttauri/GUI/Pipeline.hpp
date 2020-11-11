// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "Window_forward.hpp"
#include "../required.hpp"
#include <memory>

namespace tt {
class gui_device;

class pipeline {
public:
    Window const &window;

    pipeline(Window const &window);

    virtual ~pipeline() = default;
    pipeline(const pipeline &) = delete;
    pipeline &operator=(const pipeline &) = delete;
    pipeline(pipeline &&) = delete;
    pipeline &operator=(pipeline &&) = delete;
};

}
