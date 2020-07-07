// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/GUI/GUIDevice_forward.hpp"
#include "TTauri/GUI/Window_forward.hpp"
#include "ttauri/foundation/required.hpp"
#include <memory>

namespace tt {

class Pipeline_base {
public:
    Window const &window;
    GUIDevice *_device = nullptr;

    Pipeline_base(Window const &window);

    virtual ~Pipeline_base() = default;
    Pipeline_base(const Pipeline_base &) = delete;
    Pipeline_base &operator=(const Pipeline_base &) = delete;
    Pipeline_base(Pipeline_base &&) = delete;
    Pipeline_base &operator=(Pipeline_base &&) = delete;

    GUIDevice const &device() const {
        tt_assume(_device != nullptr);
        return *_device;
    }
};

}
