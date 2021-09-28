// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "architecture.hpp"
#include <format>

namespace tt {

#if TT_OPERATING_SYSTEM == TT_OS_WINDOWS
void _debugger_break();
#define tt_debugger_break() _debugger_break()

#elif TT_COMPILER == TT_CC_GCC || TT_COMPILER == TT_CC_CLANG
#define tt_debugger_break() __builtin_trap()

#else
#error "Not implemented"
#endif

/*! Check if the program is being debugged.
 */
bool debugger_is_present() noexcept;

[[noreturn]] void debugger_abort(std::string const &message) noexcept;

/** Abort the application.
 * @param source_file __FILE__
 * @param source_line __LINE__
 * @param fmt Message to display.
 * @param args Rest arguments to formatter
 */
template<typename... Args>
[[noreturn]] tt_no_inline void
debugger_abort(char const *source_file, int source_line, std::string_view fmt, Args &&...args) noexcept
{
    debugger_abort(std::format(fmt, std::forward<Args>(args)...));
}

[[noreturn]] tt_no_inline inline void debugger_abort(char const *source_file, int source_line) noexcept
{
    debugger_abort(source_file, source_line, "<unknown>");
}

#define tt_debugger_abort(...) ::tt::debugger_abort(__FILE__, __LINE__ __VA_OPT__(, ) __VA_ARGS__)

} // namespace tt
