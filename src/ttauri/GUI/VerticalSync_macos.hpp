// Copyright Take Vos 2019.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "VerticalSync_base.hpp"
#include <span>
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
