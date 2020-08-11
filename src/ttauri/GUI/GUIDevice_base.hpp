// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "Window.hpp"
#include "../exceptions.hpp"
#include "../numeric_cast.hpp"
#include "../bigint.hpp"
#include <unordered_set>
#include <mutex>
#include <tuple>

namespace tt {

/*! A GUIDevice that handles a set of windows.
 */
class GUIDevice_base {
public:
    enum class State {
        NO_DEVICE,
        READY_TO_DRAW,
    };

    State state = State::NO_DEVICE;

    std::string deviceName = "<no device>";
    uint32_t vendorID = 0;
    uint32_t deviceID = 0;
    uuid deviceUUID = {};

    /*! A list of windows managed by this device.
     */
    std::vector<std::unique_ptr<Window>> windows;

    std::string string() const noexcept;

    GUIDevice_base() noexcept;
    virtual ~GUIDevice_base();

    GUIDevice_base(const GUIDevice_base &) = delete;
    GUIDevice_base &operator=(const GUIDevice_base &) = delete;
    GUIDevice_base(GUIDevice_base &&) = delete;
    GUIDevice_base &operator=(GUIDevice_base &&) = delete;

    /*! Check if this device is a good match for this window.
     *
     * It is possible for a window to be created that is not presentable, in case of a headless-virtual-display,
     * however in this case it may still be able to be displayed by any device.
     *
     * \returns -1 When not viable, 0 when not presentable, postive values for increasing score.
     */
    virtual int score(Window const &window) const = 0;

    /*! Initialise the logical device.
     *
     * \param window is used as prototype for queue allocation.
     */
    virtual void initializeDevice(Window const &window);

    ssize_t getNumberOfWindows() const noexcept {
        return std::ssize(windows);
    }

    void add(std::unique_ptr<Window> window);

    void remove(Window &window) noexcept;

    void render(hires_utc_clock::time_point displayTimePoint) noexcept {
        for (auto &window: windows) {
            window->render(displayTimePoint);
        }

        ttlet new_end = std::remove_if(windows.begin(), windows.end(), [](ttlet &window) { return window->isClosed(); });
        windows.erase(new_end, windows.end());
    }
};

}
