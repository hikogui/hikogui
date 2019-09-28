// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/GUI/Window.hpp"
#include "TTauri/GUI/globals.hpp"
#include "TTauri/Diagnostic/exceptions.hpp"
#include "TTauri/Required/numeric_cast.hpp"
#include "TTauri/Required/bigint.hpp"
#include <unordered_set>
#include <mutex>
#include <tuple>

namespace TTauri::GUI {

/*! A Device that handles a set of windows.
 */
class Device_base {
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

    Device_base() noexcept;
    virtual ~Device_base();

    Device_base(const Device_base &) = delete;
    Device_base &operator=(const Device_base &) = delete;
    Device_base(Device_base &&) = delete;
    Device_base &operator=(Device_base &&) = delete;

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
        return to_signed(windows.size());
    }

    void add(std::unique_ptr<Window> window);

    void remove(Window &window) noexcept;

    void render() noexcept {
        for (auto &window: windows) {
            window->render();

            if (window->isClosed()) {
                remove(*window);
            }
        }
    }
};

}
