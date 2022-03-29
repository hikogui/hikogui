// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "loop.hpp"
#include "counters.hpp"
#include "thread.hpp"
#define IN
#define OUT
#include <WinSock2.h>
#include <Windows.h>
#include <synchapi.h>
#include <vector>
#include <utility>

namespace tt::inline v1 {

struct loop_private_win32 : loop::private_type {
    struct handle_type {
        HANDLE handle;
        int fd;
    };

    /** The handles to block on.
     *
     * The following is the order of handles:
     * - 0 : interrupt handle, for handling async/timer messages after they have been added to the queues.
     * - x : A handle, one for each socket, up to 63. XXX Implement recursive sockets-events to go beyond this limit.
     *
     */
    std::vector<handle_type> handles;
};

loop::loop() noexcept : _private(std::make_unique<loop_private_win32>()), _thread_id(current_thread_id())
{
    auto& prv = get_private<loop_private_win32>();

    auto interupt_handle = CreateEventW(NULL, FALSE, FALSE, NULL);
    if (interupt_handle == NULL) {
        tt_log_fatal("Could not create an interrupt event handle. {}", get_last_error_message());
    }

    prv.handles.emplace_back(interupt_handle, -1);
}

loop::~loop()
{
    auto& prv = get_private<loop_private_win32>();

    // Close all socket event handles.
    while (prv.handles.size() > 1) {
        auto tmp = prv.handles.back();
        if (not WSACloseEvent(tmp.handle)) {
            tt_log_error("Could not clock socket event handle for socket {}. {}", tmp.fd, get_last_error_message());
        }

        prv.handles.pop_back();
    }

    if (not CloseHandle(prv.handles.front().handle)) {
        tt_log_error("Could not close interrupt event handle. {}", get_last_error_message());
    }
}

void loop::interrupt() noexcept
{
    auto& prv = get_private<loop_private_win32>();
    if (not SetEvent(prv.handles[0].handle)) {
        tt_log_error("Could not interrupt event-loop. {}", get_last_error_message());
    }
}

int loop::max_fd() const noexcept
{
    return FD_SETSIZE - 1;
}

void loop::block_on_network(utc_nanoseconds deadline) noexcept
{
    using namespace std::chrono_literals;

    // WSAPoll() is broken wont-fix according to Microsoft.

    ::fd_set read_fds[FD_SETSIZE];
    ::fd_set write_fds[FD_SETSIZE];
    ::fd_set error_fds[FD_SETSIZE];

    FD_ZERO(read_fds);
    FD_ZERO(write_fds);
    FD_ZERO(error_fds);

    int current_max_fd = 0;
    for (ttlet& socket : _sockets) {
        current_max_fd = std::max(current_max_fd, socket.fd);

        if (any(socket.mode & select_type::read)) {
            FD_SET(socket.fd, read_fds);
        }
        if (any(socket.mode & select_type::write)) {
            FD_SET(socket.fd, write_fds);
        }
        if (any(socket.mode & select_type::error)) {
            FD_SET(socket.fd, error_fds);
        }
    }

    // Calculate the timeout for select(), select() has a likely resolution of 16ms.
    // XXX - Use timeBeginPeriod() to improve resolution of timers.
    auto wake_duration = deadline - std::chrono::utc_clock::now();
    ::timeval timeout;
    if (wake_duration > 0us) {
        timeout.tv_sec = wake_duration / 1s;
        timeout.tv_usec = wake_duration / 1us;
    } else {
        timeout.tv_sec = 0;
        timeout.tv_usec = 0;
    }

    auto count = select(current_max_fd + 1, read_fds, write_fds, error_fds, &timeout);
    if (count == SOCKET_ERROR) {
        switch (auto error_code = WSAGetLastError()) {
        case WSANOTINITIALISED: tt_log_fatal("A successful WSAStartup call must occur before using this function.");
        case WSAEFAULT:
            tt_log_fatal(
                "The Windows Sockets implementation was unable to allocate needed resources for its internal operations, or "
                "the "
                "readfds, writefds, exceptfds, or timeval parameters are not part of the user address space.");
        case WSAENETDOWN: tt_log_fatal("The network subsystem has failed.");
        case WSAEINVAL: tt_log_fatal("The time-out value is not valid, or all three descriptor parameters were null.");
        case WSAEINTR: tt_log_fatal("A blocking Windows Socket 1.1 call was canceled through WSACancelBlockingCall.");
        case WSAEINPROGRESS:
            tt_log_fatal(
                "A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback "
                "function.");
        case WSAENOTSOCK: tt_log_fatal("One of the descriptor sets contains an entry that is not a socket.");
        default: tt_log_fatal("Unknown error {} from select", error_code);
        }

    } else if (count == 0) {
        return;
    }

    auto it = _sockets.cbegin();
    auto last = _sockets.cend();

    // The callbacks may modify _sockets. So if more than 1 callback
    // needs to be called the iteration may fail.
    auto sockets_copy = decltype(_sockets){};
    if (count > 1) {
        sockets_copy = _sockets;
        it = sockets_copy.cbegin();
        last = sockets_copy.cend();
    }

    while (true) {
        tt_axiom(it != last);

        auto mode = select_type::none;
        if (any(it->mode & select_type::read) and FD_ISSET(it->fd, read_fds)) {
            mode |= select_type::read;
            --count;
        }
        if (any(it->mode & select_type::write) and FD_ISSET(it->fd, write_fds)) {
            mode |= select_type::write;
            --count;
        }
        if (any(it->mode & select_type::error) and FD_ISSET(it->fd, error_fds)) {
            mode |= select_type::error;
            --count;
        }

        if (mode != select_type::none) {
            it->callback(it->fd, mode);
        }

        if (count <= 0 or std::chrono::utc_clock::now() >= deadline) {
            // No more callbacks need to be called.
            // In case there was only one callback to be called and it modified _sockets
            // It needs to exit the loop before ++it is executed.
            return;
        }

        ++it;
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

void loop::block()
{
    auto& prv = get_private<loop_private_win32>();

    DWORD timeout_ms = 0;
    auto r = MsgWaitForMultipleObjects(prv.handles.size(), prv.handles.data(), FALSE, timeout_ms, QS_ALLINPUT);
    if (r == WAIT_FAILED) {
        tt_log_error("Failed on MsgWaitForMultipleObjects(), {}", get_last_error_message());

    } else if (r == WAIT_TIMEOUT) {
        return has_timeout;

    } else if (r == WAIT_OBJECT_0) {
        // Interrupt

    } else if (r >= WAIT_OBJECT_0 + 1 and r < WAIT_OBJECT_0 + prv.handles.size()) {
        // Socket event.
        auto &tmp = prv.handles[r - WAIT_OBJECT_0];

        WSANETWORKEVENTS events;
        if (WSAEnumNetworkEvents(tmp.fd, tmp.handle, &events) != 0) {
            throw io_error(std::format("Error during WSAEnumNetworkEvents on socket {}: {}", tmp.fd, get_last_error_message()));
        }

        auto network_events = network_event::none;
        if (events.lNetworkEvents | 

        r.socket_events.clear();
        r.socket_events.emplace_back(tmp.fd, 

    } else if (r == WAIT_OBJECT_0 + prv.handles.size()) {
        // win32 message.
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
