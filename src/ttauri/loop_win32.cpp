// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file loop_win32.cpp
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

#include "loop.hpp"
#include "counters.hpp"
#include "thread.hpp"
#include "net/network_event.hpp"
#include "net/network_event_win32.hpp"
#define IN
#define OUT
#include <WinSock2.h>
#undef IN
#undef OUT
#include <Windows.h>
#include <synchapi.h>
#include <dxgi.h>
#include <vector>
#include <utility>
#include <stop_token>
#include <thread>

namespace tt::inline v1 {

constexpr static size_t vsync_handle_idx = 0;
constexpr static size_t async_handle_idx = 1;
constexpr static size_t socket_handle_idx = 2;

struct loop_impl_win32 : loop::impl_type {
    /** event-handle to continue the vsync.
     *
     * This event handle is a manual reset event.
     *
     * - set: Use `IDXGIOutput::WaitForVBlank()` at high priority.
     * - reset: Use `WaitForSingleObject()` timeout on low priority to about 30fps.
     */
    HANDLE use_vsync_handle;

    /** Time when the last vertical blank happened.
     */
    std::atomic<utc_nanoseconds> vsync_time;

    /** The last vsync_time update was made by a call to Sleep().
     */
    bool vsync_time_from_sleep = true;

    /** pull down ratio for triggering SetEvent from WaitForVBlank.
     *
     * Format is in UQ8.8, this is done to reduce judder introduced by float precision.
     */
    std::atomic<uint16_t> pull_down = 0x100;

    /** Sub-frame count in UQ56.8 format, incremented by `pull_down` on each vertical-blank.
     *
     * This is incremented only when blocking on vertical-blank.
     */
    uint64_t sub_frame_count = 0;

    /** Frame count after pull-down.
     *
     * This is incremented only when blocking on vertical-blank.
     */
    uint64_t frame_count = 0;

    /** The handles to block on.
     *
     * The following is the order of handles:
     * - 0 : vsync event-handle
     * - 1 : async-fifo event-handle
     * - x : A handle, one for each socket.
     *
     */
    std::vector<HANDLE> handles;

    /** Socket file descriptors.
     *
     * This list contains one-to-one file descriptors with `handles`.
     * The first two file descriptors have the value -1 (for the non
     * socket handles).
     */
    std::vector<int> sockets;

    /** A list of functions to call on an event to a socket.
     */
    std::vector<std::function<void(int, network_events const&)>> socket_functions;

    /** The vsync thread.
     */
    std::jthread vsync_thread;

    /** The vsync thread handle.
     */
    HANDLE vsync_thread_handle;

    /** The current priority of the vsync thread.
     */
    int vsync_thread_priority = THREAD_PRIORITY_NORMAL;

    /** The primary monitor id.
     * As returned by os_settings::primary_monitor_id().
     */
    std::uintptr_t primary_monitor_id = 0;

    /** The DXGI Output of the primary monitor.
     */
    IDXGIOutput *primary_monitor_output = nullptr;

    /** Update the dxgi_output to point to the primary-monitor.
     *
     * @note This function is cheap if the primary-monitor does not change.
     */
    void vsync_thread_update_dxgi_output() noexcept
    {
        IDXGIFactory *factory = nullptr;
        IDXGIAdapter *adapter = nullptr;
        DXGI_OUTPUT_DESC description;

        if (not compare_store(primary_monitor_id, os_settings::primary_monitor_id())) {
            return;
        }

        if (primary_monitor_output) {
            primary_monitor_output->Release();
            primary_monitor_output = nullptr;
        }

        if (FAILED(CreateDXGIFactory(__uuidof(IDXGIFactory), (void **)&factory))) {
            tt_log_error_once("vsync:error:CreateDXGIFactory", "Could not IDXGIFactory. {}", get_last_error_message());
            goto fail;
        }

        if (FAILED(factory->EnumAdapters(0, &adapter))) {
            tt_log_error_once("vsync:error:EnumAdapters", "Could not get IDXGIAdapter. {}", get_last_error_message());
            goto fail;
        }

        if (FAILED(adapter->EnumOutputs(0, &primary_monitor_output))) {
            tt_log_error_once("vsync:error:EnumOutputs", "Could not get IDXGIOutput. {}", get_last_error_message());
            goto fail;
        }

        if (FAILED(primary_monitor_output->GetDesc(&description))) {
            tt_log_error_once("vsync:error:GetDesc", "Could not get IDXGIOutput description. {}", get_last_error_message());
            primary_monitor_output->Release();
            primary_monitor_output = nullptr;
            goto fail;
        }

        if (description.Monitor != std::bit_cast<HMONITOR>(primary_monitor_id)) {
            tt_log_error_once("vsync:error:not-primary-monitor", "DXGI primary monitor does not match desktop primary monitor");
            primary_monitor_output->Release();
            primary_monitor_output = nullptr;
            goto fail;
        }

fail:
        if (adapter) {
            adapter->Release();
        }

        if (factory) {
            factory->Release();
        }
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
        ttlet ts = time_stamp_count(time_stamp_count::inplace_with_cpu_id{});
        ttlet new_time = time_stamp_utc::make(ts);

        ttlet was_sleeping = std::exchange(vsync_time_from_sleep, on_sleep);
        ttlet old_time = vsync_time.exchange(new_time, std::memory_order::acquire);

        // If old_time was caused by sleeping it can not be used to calculate how long vsync was blocking.
        return was_sleeping ? std::chrono::nanoseconds::max() : new_time - old_time;
    }

    void vsync_thread_wait_for_vblank() noexcept
    {
        using namespace std::chrono_literals;

        vsync_thread_update_dxgi_output();

        if (primary_monitor_output and FAILED(primary_monitor_output->WaitForVBlank())) {
            tt_log_error_once("vsync:error:WaitForVBlank", "WaitForVBlank() failed. {}", get_last_error_message());
        }

        if (vsync_thread_update_time(false) < 1ms) {
            tt_log_info_once("vsync:monitor-off", "WaitForVBlank() did not block; is the monitor turned off?");
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
        sub_frame_count += pull_down.load(std::memory_order::relaxed);
        return compare_store(frame_count, sub_frame_count >> 8);
    }

    /** Change the priority of the vsync-thread.
     *
     * @note This function is cheap when requesting the same priority multiple time.
     * @param new_priority A win32 thread priority; THREAD_PRIORITY_NORMAL or THREAD_PRIORITY_TIME_CRITICAL
     */
    void vsync_thread_update_priority(int new_priority) noexcept
    {
        if (std::exchange(vsync_thread_priority, new_priority) != new_priority) {
            if (not SetThreadPriority(vsync_thread_handle, new_priority)) {
                tt_log_error_once("vsync:error:SetThreadPriority", "Could not set the vsync thread priority to {}", new_priority);
            }
        }
    }

    void vsync_thread_proc(std::stop_token stop_token) noexcept
    {
        vsync_thread_handle = GetCurrentThread();
        set_thread_name("vsync");

        while (not stop_token.stop_requested()) {
            switch (WaitForSingleObject(use_vsync_handle, 30)) {
            case WAIT_TIMEOUT:
                // When use_vsync is off wake the main loop every 30ms.
                vsync_thread_update_time(true);

                vsync_thread_update_priority(THREAD_PRIORITY_NORMAL);

                ++global_counter<"vsync:low-priority">;
                ++global_counter<"vsync:frame">;
                SetEvent(handles[vsync_handle_idx]);
                break;

            case WAIT_OBJECT_0:
                // When use_vsync is on wake the main loop based on the vertical-sync and pull_down.
                vsync_thread_update_priority(THREAD_PRIORITY_TIME_CRITICAL);

                vsync_thread_wait_for_vblank();

                if (vsync_thread_pull_down()) {
                    ++global_counter<"vsync:frame">;
                    SetEvent(handles[vsync_handle_idx]);
                }

                break;

            case WAIT_ABANDONED:
                tt_log_error_once("vsync:error:WAIT_ABANDONED", "use_vsync_handle has been abandoned.");
                ResetEvent(use_vsync_handle);
                break;

            case WAIT_FAILED:
                tt_log_error_once("vsync:error:WAIT_FAILED", "WaitForSingleObject failed. {}", get_last_error_message());
                ResetEvent(use_vsync_handle);
                break;
            }
        }
    }
};

loop::loop() : _pimpl(std::make_unique<loop_impl_win32>()), _thread_id(current_thread_id())
{
    auto& impl = get_impl<loop_impl_win32>();

    // Create an level trigger event, to use as an on/off switch.
    if (auto handle = CreateEventW(NULL, TRUE, TRUE, NULL)) {
        impl.use_vsync_handle = handle;
    } else {
        tt_log_fatal("Could not create an use-vsync handle. {}", get_last_error_message());
    }

    // Create a pulse trigger event.
    if (auto handle = CreateEventW(NULL, FALSE, FALSE, NULL)) {
        impl.handles.push_back(handle);
        impl.sockets.push_back(-1);
        impl.socket_functions.emplace_back();
    } else {
        tt_log_fatal("Could not create an vsync-event handle. {}", get_last_error_message());
    }

    // Create a pulse trigger event.
    if (auto handle = CreateEventW(NULL, FALSE, FALSE, NULL)) {
        impl.handles.push_back(handle);
        impl.sockets.push_back(-1);
        impl.socket_functions.emplace_back();
    } else {
        tt_log_fatal("Could not create an async-event handle. {}", get_last_error_message());
    }
}

loop::~loop()
{
    auto& impl = get_impl<loop_impl_win32>();

    // Close all socket event handles.
    while (impl.handles.size() >= socket_handle_idx) {
        if (not WSACloseEvent(impl.handles.back())) {
            tt_log_error("Could not clock socket event handle for socket {}. {}", impl.sockets.back(), get_last_error_message());
        }

        impl.handles.pop_back();
        impl.sockets.pop_back();
    }

    if (impl.vsync_thread.joinable()) {
        impl.vsync_thread.request_stop();
        impl.vsync_thread.join();
    }

    if (not CloseHandle(impl.handles[async_handle_idx])) {
        tt_log_error("Could not close async-event handle. {}", get_last_error_message());
    }
    if (not CloseHandle(impl.handles[vsync_handle_idx])) {
        tt_log_error("Could not close vsync-event handle. {}", get_last_error_message());
    }
    if (not CloseHandle(impl.use_vsync_handle)) {
        tt_log_error("Could not close use-vsync handle. {}", get_last_error_message());
    }
}

void loop::trigger_async() noexcept
{
    auto& impl = get_impl<loop_impl_win32>();
    if (not SetEvent(impl.handles[async_handle_idx])) {
        tt_log_error("Could not trigger async-event. {}", get_last_error_message());
    }
}

void loop::handle_vsync() noexcept
{
    // XXX Reduce the number of redraws for each window based on the refresh rate of the monitor they are located on.
    // XXX handle maximum frame rate and update vsync thread
    // XXX Update active windows more often than inactive windows.
}

void loop::handle_async() noexcept {}

void loop::handle_gui_events() noexcept
{
    MSG msg = {};
    ttlet t1 = trace<"loop:gui-events">();
    while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
        ttlet t2 = trace<"loop:gui-event">();

        if (msg.message == WM_QUIT) {
            _exit_code = narrow_cast<int>(msg.wParam);
        }

        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
}

void loop::resume_once(bool block) noexcept
{
    auto& impl = get_impl<loop_impl_win32>();

    ttlet timeout_ms = block ? 100 : 0;
    ttlet message_mask = block ? QS_ALLINPUT : 0;
    ttlet wait_r =
        MsgWaitForMultipleObjects(narrow<DWORD>(impl.handles.size()), impl.handles.data(), FALSE, timeout_ms, message_mask);
    if (wait_r == WAIT_FAILED) {
        tt_log_fatal("Failed on MsgWaitForMultipleObjects(), {}", get_last_error_message());

    } else if (wait_r == WAIT_TIMEOUT) {
        if (block) {
            // 100 ms timeout happened, this should not normally happen when vsync is working.
            tt_log_error_once("loop:error:timeout", "MsgWaitForMultipleObjects was timed-out.");
        }

    } else if (wait_r == WAIT_OBJECT_0 + vsync_handle_idx) {
        // XXX Make sure this is not starving the win32 events.
        // should we just empty the win32 events after every unblock?
        handle_vsync();

    } else if (wait_r == WAIT_OBJECT_0 + async_handle_idx) {
        // handle_async() is called after every wake-up of MsgWaitForMultipleObjects
        ;

    } else if (wait_r >= WAIT_OBJECT_0 + socket_handle_idx and wait_r < WAIT_OBJECT_0 + impl.handles.size()) {
        ttlet index = wait_r - WAIT_OBJECT_0;

        WSANETWORKEVENTS events;
        if (WSAEnumNetworkEvents(impl.sockets[index], impl.handles[index], &events) != 0) {
            switch (WSAGetLastError()) {
            case WSANOTINITIALISED: tt_log_fatal("WSAStartup was not called.");
            case WSAENETDOWN: tt_log_fatal("The network subsystem has failed.");
            case WSAEINVAL: tt_log_fatal("One of the specified parameters was invalid.");
            case WSAEINPROGRESS:
                tt_log_warning(
                    "A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback "
                    "function.");
                break;
            case WSAEFAULT: tt_log_fatal("The lpNetworkEvents parameter is not a valid part of the user address space.");
            case WSAENOTSOCK:
                // If somehow the socket was destroyed, lets just remove it.
                tt_log_error("Error during WSAEnumNetworkEvents on socket {}: {}", impl.sockets[index], get_last_error_message());
                impl.handles.erase(impl.handles.begin() + index);
                impl.sockets.erase(impl.sockets.begin() + index);
                impl.socket_functions.erase(impl.socket_functions.begin() + index);
                break;
            default: tt_no_default();
            }

        } else {
            // Because of how WSAEnumNetworkEvents() work we must only handle this specific socket.
            impl.socket_functions[index](impl.sockets[index], network_events_from_win32(events));
        }

    } else if (wait_r == WAIT_OBJECT_0 + impl.handles.size()) {
        handle_gui_events();

    } else if (wait_r >= WAIT_ABANDONED_0 and wait_r < WAIT_ABANDONED_0 + impl.handles.size()) {
        ttlet index = wait_r - WAIT_ABANDONED_0;
        if (index == vsync_handle_idx) {
            tt_log_fatal("The vsync-handle has been abandoned.");

        } else if (index == async_handle_idx) {
            tt_log_fatal("The async-handle has been abandoned.");

        } else {
            // Socket handle has been abandoned. Remove it from the handles.
            tt_log_error("The socket-handle for socket {} has been abandoned.", impl.sockets[index]);
            impl.handles.erase(impl.handles.begin() + index);
            impl.sockets.erase(impl.sockets.begin() + index);
            impl.socket_functions.erase(impl.socket_functions.begin() + index);
        }

    } else {
        tt_no_default();
    }

    // When async messages are added wait-free, the async-event is never triggered.
    // So handle messages after any kind of wake up.
    handle_async();
}

int loop::resume() noexcept
{
    // Microsoft recommends an event-loop that also renders to the screen to run at above normal priority.
    ttlet thread_handle = GetCurrentThread();

    int original_thread_priority = GetThreadPriority(thread_handle);
    if (original_thread_priority == THREAD_PRIORITY_ERROR_RETURN) {
        original_thread_priority = THREAD_PRIORITY_NORMAL;
        tt_log_error("GetThreadPriority() for loop failed {}", get_last_error_message());
    }

    if (original_thread_priority < THREAD_PRIORITY_ABOVE_NORMAL) {
        if (not SetThreadPriority(thread_handle, THREAD_PRIORITY_ABOVE_NORMAL)) {
            tt_log_error("SetThreadPriority() for loop failed {}", get_last_error_message());
        }
    }

    while (not _exit_code) {
        resume_once(true);

        // XXX when there are no: windows, async-messages, sockets or timers the loop should exit by itself.
    }

    // Set the thread priority back to what is was before resume().
    if (original_thread_priority < THREAD_PRIORITY_ABOVE_NORMAL) {
        if (not SetThreadPriority(thread_handle, original_thread_priority)) {
            tt_log_error("SetThreadPriority() for loop failed {}", get_last_error_message());
        }
    }

    return *_exit_code;
}

} // namespace tt::inline v1
