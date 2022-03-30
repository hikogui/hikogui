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

struct loop_private_win32 : loop::private_type {
    /** event-handle to continue the vsync.
     *
     * This event handle is a manual reset event.
     *
     * - set: Use `IDXGIOutput::WaitForVBlank()` at high priority.
     * - reset: Use `WaitForSingleObject()` timeout on low priority to about 30fps.
     */
    HANDLE use_vsync_handle;

    std::atomic<utc_nanoseconds> vsync_time;

    /** pull down ratio for triggering SetEvent from WaitForVBlank.
     *
     * Format is in UQ8.8, this is done to reduce judder introduced by float precision.
     */
    std::atomic<uint16_t> pull_down;

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

    std::jthread vsync_thread;

    void update_dxgi_output(IDXGIOutput *(&dxgi_output), uintptr_t& monitor_id) noexcept
    {
        IDXGIFactory *factory = nullptr;
        IDXGIAdapter *adapter = nullptr;
        DXGI_OUTPUT_DESC description;

        if (not compare_store(monitor_id, os_settings::primary_monitor_id())) {
            return;
        }

        if (dxgi_output) {
            dxgi_output->Release();
            dxgi_output = nullptr;
        }

        if (FAILED(CreateDXGIFactory(__uuidof(IDXGIFactory), (void **)&factory))) {
            tt_log_error("Could not IDXGIFactory. {}", get_last_error_message());
            goto fail;
        }

        if (FAILED(factory->EnumAdapters(0, &adapter))) {
            tt_log_error("Could not get IDXGIAdapter. {}", get_last_error_message());
            goto fail;
        }

        if (FAILED(adapter->EnumOutputs(0, &dxgi_output))) {
            tt_log_error("Could not get IDXGIOutput. {}", get_last_error_message());
            goto fail;
        }

        if (FAILED(dxgi_output->GetDesc(&description))) {
            tt_log_error("Could not get IDXGIOutput. {}", get_last_error_message());
            dxgi_output->Release();
            dxgi_output = nullptr;
            goto fail;
        }

        if (description.Monitor != std::bit_cast<HMONITOR>(monitor_id)) {
            tt_log_error("DXGI primary monitor does not match desktop primary monitor");
            dxgi_output->Release();
            dxgi_output = nullptr;
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

    std::chrono::nanoseconds update_vsync_time()
    {
        ttlet ts = time_stamp_count(time_stamp_count::inplace_with_cpu_id{});
        ttlet new_time = time_stamp_utc::make(ts);
        ttlet old_time = vsync_time.exchange(new_time, std::memory_order::acquire);
        return new_time - old_time;
    }

    void vsync_thread_proc(std::stop_token stop_token) noexcept
    {
        using namespace std::chrono_literals;

        ttlet current_thread_handle = GetCurrentThread();

        bool failed_once = false;
        bool using_vsync = false;
        uintptr_t monitor_id = 0;
        IDXGIOutput *dxgi_output = nullptr;

        // Frame-count in UQ56.8 format.
        uint64_t sub_frame_count = 0;
        uint64_t frame_count = 0;

        while (not stop_token.stop_requested()) {
            switch (WaitForSingleObject(use_vsync_handle, 30)) {
            case WAIT_TIMEOUT:
                // When use_vsync is off wake the main loop every 30ms.
                if (std::exchange(using_vsync, false) and not SetThreadPriority(current_thread_handle, THREAD_PRIORITY_NORMAL)) {
                    tt_log_error("Could not set the vsync thread priority to normal");
                }
                SetEvent(handles[vsync_handle_idx]);
                break;

            case WAIT_OBJECT_0:
                // When use_bsync is on wake the main loop based on the vertical-sync and pull_down.
                if (not std::exchange(using_vsync, true) and
                    not SetThreadPriority(current_thread_handle, THREAD_PRIORITY_TIME_CRITICAL)) {
                    tt_log_error("Could not set the vsync thread priority to time-critical");
                }

                update_dxgi_output(dxgi_output, monitor_id);

                if (dxgi_output and FAILED(dxgi_output->WaitForVBlank()) and not failed_once) {
                    failed_once = true;
                    tt_log_error("WaitForVBlank() failed. {}", get_last_error_message());
                }

                if (update_vsync_time() < 1ms) {
                    // WaitForVBlank() will not block when the monitor is turned off, it does not return an error in this case.
                    Sleep(16);
                }

                sub_frame_count += pull_down.load(std::memory_order::relaxed);
                if (compare_store(frame_count, sub_frame_count >> 8)) {
                    SetEvent(handles[vsync_handle_idx]);
                }

                break;

            case WAIT_ABANDONED:
                if (not failed_once) {
                    failed_once = true;
                    tt_log_error("use_vsync has been abandoned.");
                }
                Sleep(16);
                SetEvent(handles[vsync_handle_idx]);
                break;

            case WAIT_FAILED:
                if (not failed_once) {
                    failed_once = true;
                    tt_log_error("WaitForSingleObject failed. {}", get_last_error_message());
                }
                Sleep(16);
                SetEvent(handles[vsync_handle_idx]);
                break;
            }
        }
    }
};

loop::loop() : _private(std::make_unique<loop_private_win32>()), _thread_id(current_thread_id())
{
    auto& prv = get_private<loop_private_win32>();

    prv.use_vsync_handle = CreateEventW(NULL, TRUE, TRUE, NULL);
    if (prv.use_vsync_handle == NULL) {
        tt_log_fatal("Could not create an use-vsync handle. {}", get_last_error_message());
    }

    auto vsync_handle = CreateEventW(NULL, FALSE, FALSE, NULL);
    if (vsync_handle == NULL) {
        tt_log_fatal("Could not create an vsync-event handle. {}", get_last_error_message());
    }

    auto async_handle = CreateEventW(NULL, FALSE, FALSE, NULL);
    if (async_handle == NULL) {
        tt_log_fatal("Could not create an async-event handle. {}", get_last_error_message());
    }

    prv.handles.emplace_back(vsync_handle, -1);
    prv.handles.emplace_back(async_handle, -1);
}

loop::~loop()
{
    auto& prv = get_private<loop_private_win32>();

    // Close all socket event handles.
    while (prv.handles.size() >= socket_handle_idx) {
        if (not WSACloseEvent(prv.handles.back())) {
            tt_log_error("Could not clock socket event handle for socket {}. {}", prv.sockets.back(), get_last_error_message());
        }

        prv.handles.pop_back();
        prv.sockets.pop_back();
    }

    if (prv.vsync_thread.joinable()) {
        prv.vsync_thread.request_stop();
        prv.vsync_thread.join();
    }

    if (not CloseHandle(prv.handles[async_handle_idx])) {
        tt_log_error("Could not close async-event handle. {}", get_last_error_message());
    }
    if (not CloseHandle(prv.handles[vsync_handle_idx])) {
        tt_log_error("Could not close vsync-event handle. {}", get_last_error_message());
    }
    if (not CloseHandle(prv.use_vsync_handle)) {
        tt_log_error("Could not close use-vsync handle. {}", get_last_error_message());
    }
}

void loop::interrupt() noexcept
{
    auto& prv = get_private<loop_private_win32>();
    if (not SetEvent(prv.handles[async_handle_idx])) {
        tt_log_error("Could not trigger async-event. {}", get_last_error_message());
    }
}

void loop::handle_redraw(utc_nanoseconds deadline) noexcept
{
    // XXX Be careful not to redraw a window too often.
    // This will be called within the window's internal event loop too often.
}

void loop::resume_once(bool blocks) noexcept
{
    tt_axiom(is_same_thread());

    // Calculate next wake-up time.
    auto redraw_quota = get_redraw_quota();
    auto redraw_deadline = get_redraw_deadline();
    auto timer_deadline = blocks ? get_timer_deadline() : utc_nanoseconds{};

    // Handle network messages and block so that the CPU is yielded.
    // Make sure to wake up 5ms before the frame needs to be redrawn.
    block_on_network(std::min(redraw_deadline - redraw_quota, timer_deadline));

    // `block_on_network()` may have woken up because of a timer, for accuracy call it first.
    // But make sure we finish before the redraw must be done.
    handle_timers(redraw_deadline - redraw_quota);

    // It is important that handle_redraw is finished before the deadline.
    handle_redraw(redraw_deadline);
    if (std::chrono::utc_clock::now() > redraw_deadline) {
        ++global_counter<"loop::missed-deadline">;
    }

    // After redrawn is done, request the new dead-line.
    redraw_deadline = get_redraw_deadline();

    // Process as many gui events as possible.
    handle_gui_events(redraw_deadline - redraw_quota);

    // Process as many async calls as possible.
    handle_async(redraw_deadline - redraw_quota);
}

void loop::block(loop_events& r)
{
    r.clear();

    auto& prv = get_private<loop_private_win32>();

    DWORD timeout_ms = 30;
    auto r = MsgWaitForMultipleObjects(prv.handles.size(), prv.handles.data(), FALSE, timeout_ms, QS_ALLINPUT);
    if (r == WAIT_FAILED) {
        throw io_error(std::format("Failed on MsgWaitForMultipleObjects(), {}", get_last_error_message()));

    } else if (r == WAIT_TIMEOUT) {
        r.timeout = true;

    } else if (r == WAIT_OBJECT_0 + vsync_handle_idx) {
        // Interrupt
        r.interupt = true;

    } else if (r == WAIT_OBJECT_0 + async_handle_idx) {
        // Interrupt
        r.interupt = true;

    } else if (r >= WAIT_OBJECT_0 + socket_handle_idx and r < WAIT_OBJECT_0 + prv.handles.size()) {
        // Socket event.
        auto& tmp = prv.handles[r - WAIT_OBJECT_0];

        WSANETWORKEVENTS events;
        if (WSAEnumNetworkEvents(tmp.fd, tmp.handle, &events) != 0) {
            throw io_error(std::format("Error during WSAEnumNetworkEvents on socket {}: {}", tmp.fd, get_last_error_message()));
        }

        r.socket_events.emplace_back(tmp.fd, network_events_from_win32(events));

    } else if (r == WAIT_OBJECT_0 + prv.handles.size()) {
        r.gui_event = true;
    }
}

int loop::resume() noexcept
{
    while (not _exit_code) {
        resume_once(false);
    }

    return *_exit_code;
}

} // namespace tt::inline v1
