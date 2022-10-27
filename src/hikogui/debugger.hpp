// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "architecture.hpp"
#include <format>

namespace hi::inline v1 {

void prepare_debug_break() noexcept;

#if HI_OPERATING_SYSTEM == HI_OS_WINDOWS

#define hi_debug_break() \
    ::hi::prepare_debug_break(); \
    __debugbreak()

#else
#error Missing implementation of hi_debug_break().
#endif

#define hi_debug_abort() \
    hi_debug_break(); \
    std::terminate()

} // namespace hi::inline v1
