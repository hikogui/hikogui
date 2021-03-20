// Copyright Take Vos 2019.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "required.hpp"
#include <date/tz.h>
#include <chrono>
#include <type_traits>

namespace tt {

/** Timestamp
 */
struct hires_tai_clock {
    using rep = int64_t;
    using period = std::nano;
    using duration = std::chrono::duration<rep, period>;
    using time_point = std::chrono::time_point<hires_tai_clock>;
    static const bool is_steady = false;

	static time_point now() noexcept;

    /** Start the subsystem when needed.
     */
    static void subsystem_start() noexcept
    {
        start_subsystem(
            hires_tai_clock::_subsystem_running, false, hires_tai_clock::subsystem_init, hires_tai_clock::subsystem_deinit);
    }

private:
    static inline std::atomic<bool> _subsystem_running;
    [[nodiscard]] static bool subsystem_init() noexcept;
    static void subsystem_deinit() noexcept;
};

}
