

#pragma once

#include "ttauri/os_detect.hpp"
#include <fmt/format.h>

namespace tt {

#if  TT_OPERATING_SYSTEM == TT_OS_WINDOWS
void _debugger_break();
#define debugger_break _debugger_break()

#elif TT_COMPILER == TT_CC_GCC || TT_COMPILER == TT_CC_CLANG
#define debugger_break __builtin_trap()

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
* @param message Message to display.
* @param arg1 First argument to formatter
* @param args Rest arguments to formatter
*/
template<typename... Args>
[[noreturn]] tt_no_inline void _debugger_abort(char const *source_file, int source_line, char const *fmt, Args &&... args)
{
    std::string message;

    if constexpr (sizeof...(Args) == 0) {
        message = fmt;    
    } else {
        message = fmt::format(fmt, std::forward<Args>(args)...);
    }

    if (debugger_is_present()) {
        debugger_log("{}:{} {}", source_file, source_line, message);
        debugger_break;
    } else {
        debugger_dialogue("Aborting", "{}:{} {}", source_file, source_line, message);
    }

    std::abort();
}

#define debugger_abort(...) ::tt::_debugger_abort(__FILE__, __LINE__, __VA_ARGS__)

}
