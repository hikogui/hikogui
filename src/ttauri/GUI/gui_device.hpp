// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "Window_base.hpp"
#include "../exceptions.hpp"
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
    gui_system &system;

    enum class State {
        NO_DEVICE,
        READY_TO_DRAW,
    };

    State state = State::NO_DEVICE;

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
    virtual int score(Window_base const &window) const = 0;

    /*! Initialise the logical device.
     *
     * \param window is used as prototype for queue allocation.
     */
    virtual void initializeDevice(Window_base const &window);

    ssize_t getNumberOfWindows() const noexcept {
        return std::ssize(windows);
    }

    void add(std::shared_ptr<Window_base> window);

    void remove(Window_base &window) noexcept;

    void render(hires_utc_clock::time_point displayTimePoint) noexcept {
        ttlet lock = std::scoped_lock(gui_system_mutex);

        for (auto &window: windows) {
            window->render(displayTimePoint);
        }

        ttlet new_end = std::remove_if(windows.begin(), windows.end(), [](ttlet &window) { return window->isClosed(); });
        windows.erase(new_end, windows.end());
    }

protected:
    /** A list of windows managed by this device.
     */
    std::vector<std::shared_ptr<Window_base>> windows;
};

}
