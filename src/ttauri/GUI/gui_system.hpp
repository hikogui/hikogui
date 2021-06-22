// Copyright Take Vos 2020, 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "gfx_system.hpp"
#include "gfx_device.hpp"
#include "gui_window.hpp"
#include "gui_window_win32.hpp"
#include "gui_system_delegate.hpp"
#include "../thread.hpp"
#include "../unfair_recursive_mutex.hpp"
#include <span>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

namespace tt {

/** Graphics system
 */
class gui_system {
public:
    static inline os_handle instance;

    thread_id const thread_id;

    /*! Keep track of the numberOfWindows in the previous render cycle.
     * This way we can call closedLastWindow on the application once.
     */
    ssize_t previousNumberOfWindows = 0;

    gui_system() noexcept :
        thread_id(current_thread_id()),
        _delegate(std::make_shared<gui_system_delegate>())
    {
    }

    virtual ~gui_system() {
    }

    gui_system(const gui_system &) = delete;
    gui_system &operator=(const gui_system &) = delete;
    gui_system(gui_system &&) = delete;
    gui_system &operator=(gui_system &&) = delete;

    /** Initialize after construction.
     * Call this function directly after the constructor on the same thread.
     */
    virtual void init() {}
    virtual void deinit() {}

    void set_delegate(std::shared_ptr<gui_system_delegate> delegate) noexcept
    {
        _delegate = std::move(delegate);
    }

    gui_system_delegate &delegate() const noexcept
    {
        return *_delegate;
    }

    virtual void run_from_event_queue(std::function<void()> function) = 0;

    virtual int loop() = 0;

    virtual void exit(int exit_code) = 0;

    template<typename... Args>
    gui_window *make_window(Args &&... args)
    {
        // This function should be called from the main thread from the main loop,
        // and therefor should not have a lock on the window.
        tt_assert(thread_id == current_thread_id(), "createWindow should be called from the main thread.");
        tt_axiom(gfx_system_mutex.recurse_lock_count() == 0);

        auto window = std::make_shared<gui_window_win32>(std::forward<Args>(args)...);
        auto window_ptr = window.get();
        window->init();

        ttlet lock = std::scoped_lock(gfx_system_mutex);
        auto device = gfx_system::global().findBestDeviceForSurface(*(window->surface));
        if (!device) {
            throw gui_error("Could not find a vulkan-device matching this window");
        }

        device->add(std::move(window));
        return window_ptr;
    }

    /*! Count the number of windows managed by the GUI.
     */
    ssize_t num_windows();

    void render(hires_utc_clock::time_point displayTimePoint) {
        tt_axiom(is_gui_thread());
        ttlet lock = std::scoped_lock(gfx_system_mutex);

        gfx_system::global().render(displayTimePoint);

        ttlet currentNumberOfWindows = num_windows();
        if (currentNumberOfWindows == 0 && currentNumberOfWindows != previousNumberOfWindows) {
            gui_system::global().run_from_event_queue([this]{
                if (auto exit_code = this->delegate().last_window_closed(*this)) {
                    gui_system::global().exit(*exit_code);
                }
            });
        }
        previousNumberOfWindows = currentNumberOfWindows;
    }

    /** Check if this thread is the same as the gui thread.
     */
    [[nodiscard]] bool is_gui_thread() const noexcept
    {
        return thread_id == current_thread_id();
    }

    [[nodiscard]] static gui_system &global() noexcept
    {
        return *start_subsystem_or_terminate(_global, nullptr, subsystem_init, subsystem_deinit);
    }

private:
    static inline std::atomic<gui_system *> _global;

    std::shared_ptr<gui_system_delegate> _delegate;

    [[nodiscard]] static gui_system *subsystem_init() noexcept;
    static void subsystem_deinit() noexcept;
};

}
