// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "loop.hpp"
#include "counters.hpp"
#define IN
#define OUT
#include <WinSock2.h>
#include <Windows.h>

namespace tt::inline v1 {

void loop::interrupt() noexcept {}

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
                "The Windows Sockets implementation was unable to allocate needed resources for its internal operations, or the "
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

void loop::resume_once(bool blocks) noexcept
{
    using namespace std::chrono_literals;

    constexpr auto redraw_quota = 5ms;

    // Calculate next wake-up time.
    auto deadline = wake_time();

    // Handle network messages and block so that the CPU is yielded.
    // Make sure to wake up 5ms before the frame needs to be redrawn.
    block_on_network(deadline - redraw_quota);

    // `block_on_network()` may have woken up because of a timer.
    // But make sure we finish before the redraw must be done.
    handle_timers(deadline - redraw_quota);

    // It is important that handle_redraw is finished before the deadline.
    handle_redraw(deadline);
    if (std::chrono::utc_clock::now() > deadline) {
        ++global_counter<"loop::missed-deadline">;
    }

    // After redrawn is done, request the new dead-line.
    deadline = wake_time();

    // Process as many gui events as possible.
    handle_gui_events(deadline - redraw_quota);

    // Process as many async calls as possible.
    handle_async(deadline - redraw_quota);

}

int loop::resume() noexcept
{
    while (not _exit_code) {
        resume_once(false);
    }

    return *_exit_code;
}

} // namespace tt::inline v1