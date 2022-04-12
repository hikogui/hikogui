// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "chrono.hpp"

namespace hi::inline v1 {

class awaitable_timer {
public:
    awaitable_timer(utc_nanoseconds deadline) noexcept : _deadline(deadline) {}

    awaitable_timer(std::chrono::nanoseconds period) noexcept : awaitable_timer(std::chrono::utc_clock::now() + period) {}

private:
    utc_nanoseconds _deadline;
};

} // namespace hi::inline v1