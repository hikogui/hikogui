
#pragma once

#include "TTauri/Foundation/hires_utc_clock.hpp"
#include <functional>

namespace tt {

class VerticalSync_base {
protected:
    std::function<void(void *,hires_utc_clock::time_point)> callback;
    void *callbackData;

public:
    VerticalSync_base(std::function<void(void *,hires_utc_clock::time_point)> callback, void *callbackData) noexcept:
        callback(callback), callbackData(callbackData) {}

    ~VerticalSync_base() = default;

};

}
