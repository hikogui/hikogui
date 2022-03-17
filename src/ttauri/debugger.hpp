// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "architecture.hpp"
#include "utils.hpp"
#include <format>

namespace tt::inline v1 {

void prepare_debug_break() noexcept;

#if TT_OPERATING_SYSTEM == TT_OS_WINDOWS

#define tt_debug_break() \
    ::tt::prepare_debug_break(); \
    __debugbreak()

#else
#error Missing implementation of tt_debug_break().
#endif

#define tt_debug_abort() \
    tt_debug_break(); \
    std::terminate();

} // namespace tt::inline v1
