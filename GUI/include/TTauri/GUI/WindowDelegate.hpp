// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/GUI/Window_forward.hpp"

namespace TTauri {

class WindowDelegate {
public:
    WindowDelegate() = default;
    virtual ~WindowDelegate() = default;
    WindowDelegate(const WindowDelegate&) = delete;
    WindowDelegate& operator=(const WindowDelegate&) = delete;
    WindowDelegate(WindowDelegate&&) = delete;
    WindowDelegate& operator=(WindowDelegate&&) = delete;

    virtual void openingWindow(Window &window) = 0;
    virtual void closingWindow(const Window &window) = 0;
};

}
