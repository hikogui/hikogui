

#pragma once

#include "TTauri/Foundation/os_detect.hpp"
#include <fmt/format.h>

namespace TTauri {

#if COMPILER == CC_GCC || COMPILER == CC_CLANG
#if PROCESSOR == CPU_X64
#define debugger_break __asm__("int $3")
#elif PROCESSOR == CPU_ARM
#define debugger_break __asm__("trap")
#else
#error "Not implemented"
#endif

#elif OPERATING_SYSTEM == OS_WINDOWS
#if !defined(NDEBUG)
void _debugger_break();
#define debugger_break _debugger_break()
#else
#define debugger_break std::terminate()
#endif

#else
#error "Not implemented"
#endif

/*! Check if the program is being debugged.
 */
#if !defined(NDEBUG)
bool debugger_is_present() noexcept;
#else
constexpr bool debugger_is_present() noexcept { return false; }
#endif

void _debugger_log(char const *text) noexcept;

/*! Send a debug string to the debugger.
 */
template<typename... Args>
void debugger_log(char const *fmt, Args... args) noexcept
{
    if (sizeof...(Args) > 0) {
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
    if (sizeof...(Args) > 0) {
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
    if (sizeof...(Args) > 0) {
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

}
