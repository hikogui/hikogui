// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../win32_headers.hpp"

#include "crt_utils_intf.hpp"
#include "../telemetry/telemetry.hpp"
#include "../utility/utility.hpp"
#include "../concurrency/concurrency.hpp"
#include "../char_maps/char_maps.hpp"
#include "../time/time.hpp"
#include "../macros.hpp"
#include <cstddef>
#include <memory>
#include <cstring>
#include <string>
#include <cstdio>
#include <exception>
#include <compare>
#include <string_view>
#include <format>
#include <type_traits>
#include <chrono>

hi_export_module(hikogui.crt.crt_utils : impl);

hi_warning_push();
// C26400: Do not assign the result of an allocation or a function cal with an owner<T> return value to... (i11)
// For compatibility reasons we work with raw pointers here.
hi_warning_ignore_msvc(26400);

hi_export namespace hi { inline namespace v1 {

/** Copy a std::string to new memory.
 * The caller will have to delete [] return value.
 */
hi_export [[nodiscard]] hi_inline char *make_cstr(char const *c_str, std::size_t size = -1) noexcept
{
    if (size == -1) {
        size = std::strlen(c_str);
    }

    auto r = new char[size + 1];
    std::memcpy(r, c_str, size + 1);
    return r;
}

/** Copy a std::string to new memory.
 * The caller will have to delete [] return value.
 */
hi_export [[nodiscard]] hi_inline char *make_cstr(std::string const& s) noexcept
{
    return make_cstr(s.c_str(), s.size());
}

hi_export hi_inline std::pair<int, char **> crt_start(int, char **, void *instance, int show_cmd)
{
    // lpCmdLine does not handle UTF-8 command line properly.
    // So use GetCommandLineW() to get wide string arguments.
    // CommandLineToArgW properly unescapes the command line
    // and splits in separate arguments.
    int wargc = 0;
    auto wargv = CommandLineToArgvW(GetCommandLineW(), &wargc);
    hi_assert_not_null(wargv);

    // Convert the wchar arguments to UTF-8 and create nul terminated
    // c-strings. main() compatibility requires writable strings, so
    // we need to allocate old-style.
    char **argv = new char *[wargc + 2];
    hi_assert_not_null(argv);

    int argc = 0;
    for (; argc != wargc; ++argc) {
        argv[argc] = make_cstr(to_string(std::wstring(wargv[argc])));
    }
    LocalFree(wargv);

    // Pass nShowCmd as a the second command line argument.
    if (show_cmd == 3) {
        argv[argc++] = make_cstr("--window-state=maximize");
    } else if (show_cmd == 0 || show_cmd == 2 || show_cmd == 6 || show_cmd == 7 || show_cmd == 11) {
        argv[argc++] = make_cstr("--window-state=minimize");
    }

    // Add a nullptr to the end of the argument list.
    argv[argc] = nullptr;

    // Make sure the console is in a valid state to write text to it.
    hilet [tsc_frequency, aux_is_cpu_id] = hi::time_stamp_count::start_subsystem();

    start_system();
    if (aux_is_cpu_id) {
        hi_log_info("The AUX value from the time-stamp-count is equal to the cpu-id.");
    }
    hi_log_info("The measured frequency of the TSC is {} Hz.", tsc_frequency);

    crt_application_instance = instance;
    return {argc, argv};
}

hi_inline int crt_finish(int argc, char **argv, int exit_code)
{
    hi_assert_not_null(argv);

    shutdown_system();

    for (auto i = 0; i != argc; ++i) {
        delete[] argv[i];
    }
    delete[] argv;
    return exit_code;
}

}} // namespace hi::inline v1

hi_warning_pop();
