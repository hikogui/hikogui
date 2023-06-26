// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file utility/time_zone.hpp
 *
 * This file contains time-zone utility functions.
 */

#pragma once

#include "debugger.hpp"
#include "exception.hpp"
#include <chrono>

namespace hi { inline namespace v1 {
namespace detail {

[[nodiscard]] inline std::chrono::time_zone const *_cached_current_zone() noexcept
{
    try {
        return std::chrono::current_zone();
    } catch (...) {
        hi_debug_abort("std::chrono::current_zone() throws");
    }
}

} // namespace detail

/** Cached current time zone.
 * std::chrono::current_zone() is really slow, this keeps a cache.
 *
 * The cached current time zone is not updated when the time zone is modified
 * on the system.
 */
[[nodiscard]] inline std::chrono::time_zone const& cached_current_zone() noexcept
{
    static auto *zone = detail::_cached_current_zone();
    return *zone;
}

}} // namespace hi::v1
