// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../hires_utc_clock.hpp"
#include "../subsystem.hpp"
#include <atomic>

namespace tt {

class vertical_sync {
public:
    vertical_sync() noexcept {}

    virtual ~vertical_sync() = default;

    /** Wait for the vertical sync.
     * @return The time when the frame that is currently rendered will be displayed.
     */
    virtual hires_utc_clock::time_point wait() noexcept = 0;

    [[nodiscard]] static vertical_sync &global() noexcept
    {
        return *start_subsystem_or_terminate(_global, nullptr, subsystem_init, subsystem_deinit);
    }

private:
    static inline std::atomic<vertical_sync *> _global;

    [[nodiscard]] static vertical_sync *subsystem_init() noexcept;
    static void subsystem_deinit() noexcept;
};

}
