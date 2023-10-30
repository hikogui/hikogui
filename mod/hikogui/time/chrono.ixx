// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"

#include <chrono>

export module hikogui_time_chrono;

export namespace hi::inline v1 {

using utc_nanoseconds = std::chrono::utc_time<std::chrono::nanoseconds>;
using sys_nanoseconds = std::chrono::sys_time<std::chrono::nanoseconds>;

} // namespace hi::inline v1
