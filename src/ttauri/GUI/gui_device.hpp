// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "gui_window.hpp"
#include "gui_system_globals.hpp"
#include "../exception.hpp"
#include "../cast.hpp"
#include "../bigint.hpp"
#include "../unfair_recursive_mutex.hpp"
#include <unordered_set>
#include <mutex>
#include <tuple>

namespace tt {
class gui_system;

/*! A gui_device that handles a set of windows.
 */
class gui_device {
public:
    enum class state_type {
        no_device,
        ready_to_draw,
    };

    gui_system &system;

    state_type state = state_type::no_device;

    std::string deviceName = "<no device>";
    uint32_t vendorID = 0;
    uint32_t deviceID = 0;
    uuid deviceUUID = {};

    std::string string() const noexcept;

    gui_device(gui_system &system) noexcept;
    virtual ~gui_device();

    gui_device(const gui_device &) = delete;
    gui_device &operator=(const gui_device &) = delete;
    gui_device(gui_device &&) = delete;
    gui_device &operator=(gui_device &&) = delete;

    /*! Check if this device is a good match for this window.
     *
     * It is possible for a window to be created that is not presentable, in case of a headless-virtual-display,
     * however in this case it may still be able to be displayed by any device.
     *
     * \returns -1 When not viable, 0 when not presentable, postive values for increasing score.
     */
    virtual int score(gui_window const &window) const = 0;

    /*! Initialize the logical device.
     *
     * \param window is used as prototype for queue allocation.
     */
    virtual void initialize_device(gui_window const &window);

    ssize_t num_windows() const noexcept {
        return std::ssize(windows);
    }

    void add(std::shared_ptr<gui_window> window);

    void remove(gui_window &window) noexcept;

    void render(hires_utc_clock::time_point displayTimePoint) noexcept {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        for (auto &window: windows) {
            window->render(displayTimePoint);
        }

        ttlet new_end = std::remove_if(windows.begin(), windows.end(), [](ttlet &window) { return window->is_closed(); });
        windows.erase(new_end, windows.end());
    }

protected:
    /** A list of windows managed by this device.
     */
    std::vector<std::shared_ptr<gui_window>> windows;
};

}
