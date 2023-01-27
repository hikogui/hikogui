// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file loop_win32_impl.cpp
 *
 * This is the win32 implementation of the main loop.
 *
 * It works as follows:
 *
 * The main-loop primarily blocks on `MsgWaitForMultipleObjects()` which waits on
 * handles and on the win32 message queue. There are quite a few types of handles
 * that it can block on, but in this case we use it for Events and on winsock2 select events.
 *
 * `MsgWaitForMultipleObjects()` will release on only a single of those handles at a time
 * and its priority is based on the order of the handles.
 *
 * We will use the first handle for an event triggered by `IDXGIOutput::WaitForVBlank()` running
 * on a separate high priority thread; using `SetEvent()` to trigger the event.
 * The desktop-window-manager (DWM) is refreshed on the vsync of the primary monitor,
 * also for windows running on another monitor. For performance reasons `SetEvent()` may be
 * frequency divided based on the window that is located on a monitor with the highest refresh rate.
 *
 * The second handle is for triggering processing of the asynchronous fifo. When adding asynchronous
 * calls the caller can specify if the call needs to processed immediately (non-wait-free), or
 * at the next natural release of `MsgWaitForMultipleObjects()` (wait-free).
 *
 * For networking we use a handle for each socket, subscribed and updated using `WSAEventSelect()`.
 * Since `MsgWaitForMultipleObjects()` can only handle up to 64 handles, for a high number of sockets
 * This needs to be handled as a tree of threads, each blocking on up the 64 sockets and triggering
 * the parent using an event.
 *
 * Timers are added directly on the win32 message queue.
 *
 */

#include "utility/win32_headers.hpp"

#include "loop.hpp"
#include "defer.hpp"
#include "counters.hpp"
#include "trace.hpp"
#include "utility/module.hpp"
#include "log.hpp"
#include "GUI/gui_window.hpp"
#include "net/network_event.hpp"
#include "net/network_event_win32.hpp"
#include <vector>
#include <utility>
#include <stop_token>
#include <thread>
#include <chrono>

namespace hi::inline v1 {

class loop_impl_win32 final : public loop::impl_type {
public:
    loop_impl_win32() : loop::impl_type()
    {
        // Create an level trigger event, to use as an on/off switch.
        if (auto handle = CreateEventW(NULL, TRUE, TRUE, NULL)) {
            _use_vsync_handle = handle;
        } else {
            hi_log_fatal("Could not create an use-vsync handle. {}", get_last_error_message());
        }

        // Create a pulse trigger event.
        if (auto handle = CreateEventW(NULL, FALSE, FALSE, NULL)) {
            _handles.push_back(handle);
            _sockets.push_back(-1);
            _socket_functions.emplace_back();
        } else {
            hi_log_fatal("Could not create an vsync-event handle. {}", get_last_error_message());
        }

        // Create a pulse trigger event.
        if (auto handle = CreateEventW(NULL, FALSE, FALSE, NULL)) {
            _handles.push_back(handle);
            _sockets.push_back(-1);
            _socket_functions.emplace_back();
        } else {
            hi_log_fatal("Could not create an async-event handle. {}", get_last_error_message());
        }
    }

    ~loop_impl_win32()
    {
        // Close all socket event handles.
        while (_handles.size() > _socket_handle_idx) {
            if (not WSACloseEvent(_handles.back())) {
                hi_log_error("Could not clock socket event handle for socket {}. {}", _sockets.back(), get_last_error_message());
            }

            _handles.pop_back();
            _sockets.pop_back();
        }

        if (_vsync_thread.joinable()) {
            _vsync_thread.request_stop();
            _vsync_thread.join();
        }

        if (not CloseHandle(_handles[_function_handle_idx])) {
            hi_log_error("Could not close async-event handle. {}", get_last_error_message());
        }
        if (not CloseHandle(_handles[_vsync_handle_idx])) {
            hi_log_error("Could not close vsync-event handle. {}", get_last_error_message());
        }
        if (not CloseHandle(_use_vsync_handle)) {
            hi_log_error("Could not close use-vsync handle. {}", get_last_error_message());
        }
    }

    void set_maximum_frame_rate(double frame_rate) noexcept override
    {
        hi_axiom(on_thread());
    }

    void add_window(std::weak_ptr<gui_window> window) noexcept override
    {
        hi_axiom(on_thread());
        _windows.push_back(std::move(window));

        // Startup the vsync thread once there is a window.
        if (not _vsync_thread.joinable()) {
            _vsync_thread = std::jthread{[this](std::stop_token token) {
                return vsync_thread_proc(std::move(token));
            }};
        }
    }

    void add_socket(int fd, network_event event_mask, std::function<void(int, network_events const&)> f) override
    {
        hi_axiom(on_thread());
    }

    void remove_socket(int fd) override
    {
        hi_axiom(on_thread());
    }

    int resume(std::stop_token stop_token) noexcept override
    {
        // Once the loop is resuming, all other calls should be from the same thread.
        _thread_id = current_thread_id();

        // Microsoft recommends an event-loop that also renders to the screen to run at above normal priority.
        hilet thread_handle = GetCurrentThread();

        int original_thread_priority = GetThreadPriority(thread_handle);
        if (original_thread_priority == THREAD_PRIORITY_ERROR_RETURN) {
            original_thread_priority = THREAD_PRIORITY_NORMAL;
            hi_log_error("GetThreadPriority() for loop failed {}", get_last_error_message());
        }

        if (is_main and original_thread_priority < THREAD_PRIORITY_ABOVE_NORMAL) {
            if (not SetThreadPriority(thread_handle, THREAD_PRIORITY_ABOVE_NORMAL)) {
                hi_log_error("SetThreadPriority() for loop failed {}", get_last_error_message());
            }
        }

        _exit_code = {};
        while (not _exit_code) {
            resume_once(true);

            if (stop_token.stop_possible()) {
                if (stop_token.stop_requested()) {
                    // Stop immediately when stop is requested.
                    _exit_code = 0;
                }
            } else {
                if (_windows.empty() and _function_fifo.empty() and _function_timer.empty() and
                    _handles.size() <= _socket_handle_idx) {
                    // If there is not stop token, then exit when there are no more resources to wait on.
                    _exit_code = 0;
                }
            }
        }

        // Set the thread priority back to what is was before resume().
        if (is_main and original_thread_priority < THREAD_PRIORITY_ABOVE_NORMAL) {
            if (not SetThreadPriority(thread_handle, original_thread_priority)) {
                hi_log_error("SetThreadPriority() for loop failed {}", get_last_error_message());
            }
        }

        _thread_id = 0;
        return *_exit_code;
    }

    void resume_once(bool block) noexcept override
    {
        using namespace std::chrono_literals;

        hi_axiom(on_thread());

        auto current_time = std::chrono::utc_clock::now();
        auto timeout = std::chrono::duration_cast<std::chrono::milliseconds>(_function_timer.current_deadline() - current_time);

        timeout = std::clamp(timeout, 0ms, 100ms);
        hilet timeout_ms = narrow_cast<DWORD>(timeout / 1ms);

        // Only handle win32 messages when blocking.
        // Since non-blocking is called from the win32 message-pump, we do not want to re-enter the loop.
        hilet message_mask = is_main and block ? QS_ALLINPUT : 0;

        hilet wait_r =
            MsgWaitForMultipleObjects(narrow_cast<DWORD>(_handles.size()), _handles.data(), FALSE, timeout_ms, message_mask);

        if (wait_r == WAIT_FAILED) {
            hi_log_fatal("Failed on MsgWaitForMultipleObjects(), {}", get_last_error_message());

        } else if (wait_r == WAIT_TIMEOUT) {
            // handle_functions() and handle_timers() is called after every wake-up of MsgWaitForMultipleObjects
            ;

        } else if (wait_r == WAIT_OBJECT_0 + _vsync_handle_idx) {
            // XXX Make sure this is not starving the win32 events.
            // should we just empty the win32 events after every unblock?
            handle_vsync();

        } else if (wait_r == WAIT_OBJECT_0 + _function_handle_idx) {
            // handle_functions() and handle_timers() is called after every wake-up of MsgWaitForMultipleObjects
            ;

        } else if (wait_r >= WAIT_OBJECT_0 + _socket_handle_idx and wait_r < WAIT_OBJECT_0 + _handles.size()) {
            hilet index = wait_r - WAIT_OBJECT_0;

            WSANETWORKEVENTS events;
            if (WSAEnumNetworkEvents(_sockets[index], _handles[index], &events) != 0) {
                switch (WSAGetLastError()) {
                case WSANOTINITIALISED:
                    hi_log_fatal("WSAStartup was not called.");
                case WSAENETDOWN:
                    hi_log_fatal("The network subsystem has failed.");
                case WSAEINVAL:
                    hi_log_fatal("One of the specified parameters was invalid.");
                case WSAEINPROGRESS:
                    hi_log_warning(
                        "A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a "
                        "callback "
                        "function.");
                    break;
                case WSAEFAULT:
                    hi_log_fatal("The lpNetworkEvents parameter is not a valid part of the user address space.");
                case WSAENOTSOCK:
                    // If somehow the socket was destroyed, lets just remove it.
                    hi_log_error("Error during WSAEnumNetworkEvents on socket {}: {}", _sockets[index], get_last_error_message());
                    _handles.erase(_handles.begin() + index);
                    _sockets.erase(_sockets.begin() + index);
                    _socket_functions.erase(_socket_functions.begin() + index);
                    break;
                default:
                    hi_no_default();
                }

            } else {
                // Because of how WSAEnumNetworkEvents() work we must only handle this specific socket.
                _socket_functions[index](_sockets[index], network_events_from_win32(events));
            }

        } else if (wait_r == WAIT_OBJECT_0 + _handles.size()) {
            handle_gui_events();

        } else if (wait_r >= WAIT_ABANDONED_0 and wait_r < WAIT_ABANDONED_0 + _handles.size()) {
            hilet index = wait_r - WAIT_ABANDONED_0;
            if (index == _vsync_handle_idx) {
                hi_log_fatal("The vsync-handle has been abandoned.");

            } else if (index == _function_handle_idx) {
                hi_log_fatal("The async-handle has been abandoned.");

            } else {
                // Socket handle has been abandoned. Remove it from the handles.
                hi_log_error("The socket-handle for socket {} has been abandoned.", _sockets[index]);
                _handles.erase(_handles.begin() + index);
                _sockets.erase(_sockets.begin() + index);
                _socket_functions.erase(_socket_functions.begin() + index);
            }

        } else {
            hi_no_default();
        }

        // Make sure timers are handled first, possibly they are time critical.
        handle_timers();

        // When functions are added wait-free, the function-event is never triggered.
        // So handle messages after any kind of wake up.
        handle_functions();
    }

private:
    struct socket_type {
        int fd;
        network_event mode;
        std::function<void(int, network_events const&)> callback;
    };

    constexpr static size_t _vsync_handle_idx = 0;
    constexpr static size_t _function_handle_idx = 1;
    constexpr static size_t _socket_handle_idx = 2;

    /** event-handle to continue the vsync.
     *
     * This event handle is a manual reset event.
     *
     * - set: Use `IDXGIOutput::WaitForVBlank()` at high priority.
     * - reset: Use `WaitForSingleObject()` timeout on low priority to about 30fps.
     */
    HANDLE _use_vsync_handle;

    /** Time when the last vertical blank happened.
     */
    std::atomic<utc_nanoseconds> _vsync_time;

    /** The last vsync_time update was made by a call to Sleep().
     */
    bool _vsync_time_from_sleep = true;

    /** pull down ratio for triggering SetEvent from WaitForVBlank.
     *
     * Format is in UQ8.8, this is done to reduce judder introduced by float precision.
     */
    std::atomic<uint16_t> _pull_down = 0x100;

    /** Sub-frame count in UQ56.8 format, incremented by `pull_down` on each vertical-blank.
     *
     * This is incremented only when blocking on vertical-blank.
     */
    uint64_t _sub_frame_count = 0;

    /** Frame count after pull-down.
     *
     * This is incremented only when blocking on vertical-blank.
     */
    uint64_t _frame_count = 0;

    /** The handles to block on.
     *
     * The following is the order of handles:
     * - 0 : vsync event-handle
     * - 1 : async-fifo event-handle
     * - x : A handle, one for each socket.
     *
     */
    std::vector<HANDLE> _handles;

    /** Socket file descriptors.
     *
     * This list contains one-to-one file descriptors with `handles`.
     * The first two file descriptors have the value -1 (for the non
     * socket handles).
     */
    std::vector<int> _sockets;

    /** A list of functions to call on an event to a socket.
     */
    std::vector<std::function<void(int, network_events const&)>> _socket_functions;

    /** The vsync thread.
     */
    std::jthread _vsync_thread;

    /** The vsync thread handle.
     */
    HANDLE _vsync_thread_handle;

    /** The current priority of the vsync thread.
     */
    int _vsync_thread_priority = THREAD_PRIORITY_NORMAL;

    /** The primary monitor id.
     * As returned by os_settings::primary_monitor_id().
     */
    std::uintptr_t _primary_monitor_id = 0;

    /** The DXGI Output of the primary monitor.
     */
    IDXGIOutput *_primary_monitor_output = nullptr;

    void notify_has_send() noexcept override
    {
        if (not SetEvent(_handles[_function_handle_idx])) {
            hi_log_error("Could not trigger async-event. {}", get_last_error_message());
        }
    }

    /** Call elapsed timers.
     *
     * @param deadline The deadline before all timers must be handled before moving on.
     */
    void handle_vsync() noexcept
    {
        // XXX Reduce the number of redraws for each window based on the refresh rate of the monitor they are located on.
        // XXX handle maximum frame rate and update vsync thread
        // XXX Update active windows more often than inactive windows.

        if (not _vsync_thread.joinable()) {
            // Fallback for the vsync_time advancing when the vsync thread is not running.
            _vsync_time.store(std::chrono::utc_clock::now());
        }

        hilet display_type = _vsync_time.load(std::memory_order::relaxed) + std::chrono::milliseconds(30);

        for (auto& window : _windows) {
            if (auto window_ = window.lock()) {
                window_->render(display_type);
            }
        }

        std::erase_if(_windows, [](auto& window) {
            return window.expired();
        });

        if (_windows.empty()) {
            // Stop the vsync thread when there are no more windows.
            if (_vsync_thread.joinable()) {
                _vsync_thread.request_stop();
            }
        }
    }

    /** Handle all function calls.
     *
     * @param deadline The deadline before all calls must be executed before moving on.
     */
    void handle_functions() noexcept
    {
        _function_fifo.run_all();
    }

    void handle_timers() noexcept
    {
        _function_timer.run_all(std::chrono::utc_clock::now());
    }

    /** Handle the gui events.
     *
     * @param deadline The deadline before all gui-events are handled before moving on.
     */
    void handle_gui_events() noexcept
    {
        MSG msg = {};
        hilet t1 = trace<"loop:gui-events">();
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE | PM_NOYIELD)) {
            hilet t2 = trace<"loop:gui-event">();

            if (msg.message == WM_QUIT) {
                _exit_code = narrow_cast<int>(msg.wParam);
                continue;
            }

            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }

    /** Update the dxgi_output to point to the primary-monitor.
     *
     * @note This function is cheap if the primary-monitor does not change.
     */
    void vsync_thread_update_dxgi_output() noexcept
    {
        if (not compare_store(_primary_monitor_id, os_settings::primary_monitor_id())) {
            return;
        }

        if (_primary_monitor_output) {
            _primary_monitor_output->Release();
            _primary_monitor_output = nullptr;
        }

        IDXGIFactory *factory = nullptr;
        if (FAILED(CreateDXGIFactory(__uuidof(IDXGIFactory), (void **)&factory))) {
            hi_log_error_once("vsync:error:CreateDXGIFactory", "Could not IDXGIFactory. {}", get_last_error_message());
            return;
        }
        hi_assert_not_null(factory);
        auto d1 = defer([&] {
            factory->Release();
        });

        IDXGIAdapter *adapter = nullptr;
        if (FAILED(factory->EnumAdapters(0, &adapter))) {
            hi_log_error_once("vsync:error:EnumAdapters", "Could not get IDXGIAdapter. {}", get_last_error_message());
            return;
        }
        hi_assert_not_null(adapter);
        auto d2 = defer([&] {
            adapter->Release();
        });

        if (FAILED(adapter->EnumOutputs(0, &_primary_monitor_output))) {
            hi_log_error_once("vsync:error:EnumOutputs", "Could not get IDXGIOutput. {}", get_last_error_message());
            return;
        }

        DXGI_OUTPUT_DESC description;
        if (FAILED(_primary_monitor_output->GetDesc(&description))) {
            hi_log_error_once("vsync:error:GetDesc", "Could not get IDXGIOutput description. {}", get_last_error_message());
            _primary_monitor_output->Release();
            _primary_monitor_output = nullptr;
            return;
        }

        if (description.Monitor != std::bit_cast<HMONITOR>(_primary_monitor_id)) {
            hi_log_error_once("vsync:error:not-primary-monitor", "DXGI primary monitor does not match desktop primary monitor");
            _primary_monitor_output->Release();
            _primary_monitor_output = nullptr;
            return;
        }

        d2.cancel();
        d1.cancel();
    }

    /** Update the `vsync_time`.
     *
     * This function should be called directly after a vsync or sleep
     * to update the time when the last vsync happened. The `vsync_time`
     * is used to calculate the time when the next frame is displayed on the screen.
     *
     * @param on_sleep Set to true when this function was called after sleeping.
     * @return The duration since the last vsync. Used to determine if vsync didn't block.
     */
    std::chrono::nanoseconds vsync_thread_update_time(bool on_sleep)
    {
        hilet ts = time_stamp_count(time_stamp_count::inplace_with_cpu_id{});
        hilet new_time = time_stamp_utc::make(ts);

        hilet was_sleeping = std::exchange(_vsync_time_from_sleep, on_sleep);
        hilet old_time = _vsync_time.exchange(new_time, std::memory_order::acquire);

        // If old_time was caused by sleeping it can not be used to calculate how long vsync was blocking.
        return was_sleeping ? std::chrono::nanoseconds::max() : new_time - old_time;
    }

    void vsync_thread_wait_for_vblank() noexcept
    {
        using namespace std::chrono_literals;

        vsync_thread_update_dxgi_output();

        if (_primary_monitor_output and FAILED(_primary_monitor_output->WaitForVBlank())) {
            hi_log_error_once("vsync:error:WaitForVBlank", "WaitForVBlank() failed. {}", get_last_error_message());
        }

        if (vsync_thread_update_time(false) < 1ms) {
            hi_log_info_once("vsync:monitor-off", "WaitForVBlank() did not block; is the monitor turned off?");
            Sleep(16);

            // Fixup the time after the fallback sleep.
            vsync_thread_update_time(true);
        } else {
            ++global_counter<"vsync:vertical-blank">;
        }
    }

    /** The pull-down algorithm
     *
     * Handles pull-down from the frame rate of the primary monitor to the maximum frame-rate of all windows.
     * The calculation here uses fixed-point to get a fixed pattern/cadence of frame updates.
     *
     * @return True if the frame needs to be updated.
     */
    [[nodiscard]] bool vsync_thread_pull_down() noexcept
    {
        _sub_frame_count += _pull_down.load(std::memory_order::relaxed);
        return compare_store(_frame_count, _sub_frame_count >> 8);
    }

    /** Change the priority of the vsync-thread.
     *
     * @note This function is cheap when requesting the same priority multiple time.
     * @param new_priority A win32 thread priority; THREAD_PRIORITY_NORMAL or THREAD_PRIORITY_TIME_CRITICAL
     */
    void vsync_thread_update_priority(int new_priority) noexcept
    {
        if (std::exchange(_vsync_thread_priority, new_priority) != new_priority) {
            if (not SetThreadPriority(_vsync_thread_handle, new_priority)) {
                hi_log_error_once("vsync:error:SetThreadPriority", "Could not set the vsync thread priority to {}", new_priority);
            }
        }
    }

    void vsync_thread_proc(std::stop_token stop_token) noexcept
    {
        _vsync_thread_handle = GetCurrentThread();
        set_thread_name("vsync");

        while (not stop_token.stop_requested()) {
            switch (WaitForSingleObject(_use_vsync_handle, 30)) {
            case WAIT_TIMEOUT:
                // When use_vsync is off wake the main loop every 30ms.
                vsync_thread_update_time(true);

                vsync_thread_update_priority(THREAD_PRIORITY_NORMAL);

                ++global_counter<"vsync:low-priority">;
                ++global_counter<"vsync:frame">;
                SetEvent(_handles[_vsync_handle_idx]);
                break;

            case WAIT_OBJECT_0:
                // When use_vsync is on wake the main loop based on the vertical-sync and pull_down.
                vsync_thread_update_priority(THREAD_PRIORITY_TIME_CRITICAL);

                vsync_thread_wait_for_vblank();

                if (vsync_thread_pull_down()) {
                    ++global_counter<"vsync:frame">;
                    SetEvent(_handles[_vsync_handle_idx]);
                }

                break;

            case WAIT_ABANDONED:
                hi_log_error_once("vsync:error:WAIT_ABANDONED", "use_vsync_handle has been abandoned.");
                ResetEvent(_use_vsync_handle);
                break;

            case WAIT_FAILED:
                hi_log_error_once("vsync:error:WAIT_FAILED", "WaitForSingleObject failed. {}", get_last_error_message());
                ResetEvent(_use_vsync_handle);
                break;
            }
        }
    }
};

loop::loop() : _pimpl(std::make_unique<loop_impl_win32>()) {}

} // namespace hi::inline v1
