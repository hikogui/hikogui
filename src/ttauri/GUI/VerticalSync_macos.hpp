// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/GUI/VerticalSync_base.hpp"
#include <nonstd/span>
#include <thread>

typedef struct __CVDisplayLink CVDisplayLinkRef;

namespace tt {

class VerticalSync_macos final : public VerticalSync_base {
private:
    CVDisplayLinkRef *displayLink;

public:
    VerticalSync_macos(std::function<void(void *,hires_utc_clock::time_point)> callback, void *callbackData) noexcept;
    ~VerticalSync_macos();
};

}
