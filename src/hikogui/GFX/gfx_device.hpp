// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "gfx_system_globals.hpp"
#include "gfx_surface.hpp"
#include "../utility/utility.hpp"
#include "../numeric/module.hpp"
#include "../macros.hpp"
#include <unordered_set>
#include <mutex>
#include <tuple>

namespace hi::inline v1 {
class gfx_system;

/*! A gfx_device that handles a set of windows.
 */
class gfx_device {
public:
    gfx_system& system;

    std::string deviceName = "<no device>";
    uint32_t vendorID = 0;
    uint32_t deviceID = 0;
    uuid deviceUUID = {};

    virtual ~gfx_device() = default;
    gfx_device(const gfx_device&) = delete;
    gfx_device& operator=(const gfx_device&) = delete;
    gfx_device(gfx_device&&) = delete;
    gfx_device& operator=(gfx_device&&) = delete;
    gfx_device(gfx_system& system) noexcept : system(system) {}

    std::string string() const noexcept
    {
        hilet lock = std::scoped_lock(gfx_system_mutex);

        return std::format("{0:04x}:{1:04x} {2} {3}", vendorID, deviceID, deviceName, deviceUUID.uuid_string());
    }

    /*! Check if this device is a good match for this window.
     *
     * It is possible for a window to be created that is not presentable, in case of a headless-virtual-display,
     * however in this case it may still be able to be displayed by any device.
     *
     * \returns -1 When not viable, 0 when not presentable, positive values for increasing score.
     */
    virtual int score(gfx_surface const& surface) const = 0;

    virtual void log_memory_usage() const noexcept {}
};

} // namespace hi::inline v1
