// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "../required.hpp"
#include <memory>

namespace tt {
class Window_base;

class pipeline {
public:
    Window_base const &window;

    pipeline(Window_base const &window);

    virtual ~pipeline() = default;
    pipeline(const pipeline &) = delete;
    pipeline &operator=(const pipeline &) = delete;
    pipeline(pipeline &&) = delete;
    pipeline &operator=(pipeline &&) = delete;
};

}
