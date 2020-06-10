
#pragma once

#include "TTauri/Foundation/hires_utc_clock.hpp"
#include <functional>

namespace TTauri {

class VerticalSync_base {
public:
    VerticalSync_base(std::function<void(void *,hires_utc_clock::time_point)> callback, void *callbackData) noexcept;
    ~VerticalSync_base();

};

}
