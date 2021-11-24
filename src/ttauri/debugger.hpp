// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "architecture.hpp"
#include <format>

namespace tt::inline v1 {

void _prepare_debug_break(char const *source_file, int source_line, std::string const &message) noexcept;

/** Prepare fallback for breaking in the debugger.
 *
 * @param source_file __FILE__
 * @param source_line __LINE__
 * @param fmt Message to display.
 * @param args Rest arguments to formatter
 */
template<typename... Args>
tt_no_inline void prepare_debug_break(char const *source_file, int source_line, std::string_view fmt, Args &&...args) noexcept
{
    _prepare_debug_break(source_file, source_line, std::format(fmt, std::forward<Args>(args)...));
}

/** Prepare fallback for breaking in the debugger.
 *
 * @param source_file __FILE__
 * @param source_line __LINE__
 */
tt_no_inline inline void prepare_debug_break(char const *source_file, int source_line) noexcept
{
    _prepare_debug_break(source_file, source_line, "<unknown>");
}

#if TT_OPERATING_SYSTEM == TT_OS_WINDOWS

#define tt_debug_break(...) ::tt::prepare_debug_break(__FILE__, __LINE__ __VA_OPT__(, ) __VA_ARGS__); __debugbreak()

#else
#error Missing implementation of tt_debug_break().
#endif

#define tt_debug_abort(...) tt_debug_break(__VA_ARGS__); std::terminate();

} // namespace tt::inline v1
