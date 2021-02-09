// Copyright Take Vos 2019.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)


#pragma once

#include "os_detect.hpp"
#include <fmt/format.h>

namespace tt {

#if  TT_OPERATING_SYSTEM == TT_OS_WINDOWS
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

void _debugger_log(char const *text) noexcept;

/*! Send a debug string to the debugger.
 */
template<typename... Args>
void debugger_log(char const *fmt, Args... args) noexcept
{
    if constexpr (sizeof...(Args) > 0) {
        _debugger_log(fmt::format(fmt, std::forward<Args>(args)...).data());
    } else {
        _debugger_log(fmt);
    }
}

/*! Send a debug string to the debugger.
 */
template<typename... Args>
void debugger_log(std::string fmt, Args... args) noexcept
{
    if constexpr (sizeof...(Args) > 0) {
        _debugger_log(fmt::format(fmt, std::forward<Args>(args)...).data());
    } else {
        _debugger_log(fmt.data());
    }
}

/*! Open a dialogue window.
 */
void _debugger_dialogue(char const *caption, char const *message);

/*! Send a debug string to the debugger.
 */
template<typename... Args>
void debugger_dialogue(char const *caption, char const *fmt, Args... args) noexcept
{
    if constexpr (sizeof...(Args) > 0) {
        _debugger_dialogue(caption, fmt::format(fmt, std::forward<Args>(args)...).data());
    } else {
        _debugger_dialogue(caption, fmt);
    }
}

/*! Send a debug string to the debugger.
 */
template<typename... Args>
void debugger_dialogue(std::string caption, std::string fmt, Args... args) noexcept
{
    if (sizeof...(Args) > 0) {
        _debugger_dialogue(caption.data(), fmt::format(fmt, std::forward<Args>(args)...).data());
    } else {
        _debugger_dialogue(caption.data(), fmt.data());
    }
}


/** Abort the application.
* @param source_file __FILE__
* @param source_line __LINE__
* @param fmt Message to display.
* @param args Rest arguments to formatter
*/
template<typename... Args>
[[noreturn]] tt_no_inline void debugger_abort(char const *source_file, int source_line, std::string_view fmt, Args &&... args)
{
    std::string message;

    if constexpr (sizeof...(Args) == 0) {
        message = fmt;
    } else {
        message = fmt::format(fmt, std::forward<Args>(args)...);
    }

    if (debugger_is_present()) {
        debugger_log("{}:{} {}", source_file, source_line, message);
        tt_debugger_break();
    } else {
        debugger_dialogue("Aborting", "{}:{} {}", source_file, source_line, message);
    }

    std::abort();
}

[[noreturn]] tt_no_inline inline void debugger_abort(char const *source_file, int source_line)
{
    debugger_abort(source_file, source_line, "<unknown>");
}

#define tt_debugger_abort(...) ::tt::debugger_abort(__FILE__, __LINE__ __VA_OPT__(,) __VA_ARGS__)

}
