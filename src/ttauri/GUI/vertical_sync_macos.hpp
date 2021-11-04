// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "vertical_sync_base.hpp"
#include <span>
#include <thread>

typedef struct __CVDisplayLink CVDisplayLinkRef;

namespace tt::inline v1 {

class vertical_sync_macos final : public vertical_sync_base {
private:
    CVDisplayLinkRef *displayLink;

public:
    vertical_sync_macos(std::function<void(void *, utc_nanoseconds)> callback, void *callbackData) noexcept;
    ~vertical_sync_macos();
};

} // namespace tt::inline v1
