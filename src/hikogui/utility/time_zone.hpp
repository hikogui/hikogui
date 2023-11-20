// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file utility/time_zone.hpp
 *
 * This file contains time-zone utility functions.
 */

#pragma once

#include "../macros.hpp"
#include "terminate.hpp"
#include "exception.hpp"
#include <chrono>

hi_export_module(hikogui.utility.time_zone);

hi_export namespace hi { inline namespace v1 {
namespace detail {

[[nodiscard]] hi_inline std::chrono::time_zone const *_cached_current_zone() noexcept
{
    try {
        return std::chrono::current_zone();
    } catch (...) {
        hi_no_default("std::chrono::current_zone() throws");
    }
}

} // namespace detail

/** Cached current time zone.
 * std::chrono::current_zone() is really slow, this keeps a cache.
 *
 * The cached current time zone is not updated when the time zone is modified
 * on the system.
 */
[[nodiscard]] hi_inline std::chrono::time_zone const& cached_current_zone() noexcept
{
    static auto *zone = detail::_cached_current_zone();
    return *zone;
}

}} // namespace hi::v1
