// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "gfx_device.hpp"
#include "../utility/module.hpp"
#include <span>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

namespace hi::inline v1 {
class gfx_surface;

/** Graphics system
 */
class gfx_system {
public:
    //! List of all devices.
    std::vector<std::shared_ptr<gfx_device>> devices;

    gfx_system() noexcept;

    virtual ~gfx_system();

    gfx_system(const gfx_system &) = delete;
    gfx_system &operator=(const gfx_system &) = delete;
    gfx_system(gfx_system &&) = delete;
    gfx_system &operator=(gfx_system &&) = delete;

    /** Initialize after construction.
     * Call this function directly after the constructor on the same thread.
     */
    virtual void init(){};
    virtual void deinit(){};

    [[nodiscard]] virtual std::unique_ptr<gfx_surface> make_surface(os_handle instance, void *os_window) const noexcept = 0;

    void log_memory_usage() const noexcept {
        for (hilet &device: devices) {
            device->log_memory_usage();
        }
    }

    gfx_device *find_best_device_for_surface(gfx_surface const &surface);
};

} // namespace hi::inline v1
