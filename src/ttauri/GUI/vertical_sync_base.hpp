// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../hires_utc_clock.hpp"
#include <functional>

namespace tt {

class vertical_sync_base {
protected:
    std::function<void(void *,hires_utc_clock::time_point)> callback;
    void *callbackData;

public:
    vertical_sync_base(std::function<void(void *,hires_utc_clock::time_point)> callback, void *callbackData) noexcept:
        callback(callback), callbackData(callbackData) {}

    ~vertical_sync_base() = default;

};

}
