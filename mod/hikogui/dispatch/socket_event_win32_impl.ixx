// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"
#include "../win32_headers.hpp"


#include <exception>

export module hikogui_dispatch_socket_event : impl;
import : intf;
import hikogui_utility;

export namespace hi::inline v1 {

[[nodiscard]] constexpr socket_event socket_event_from_win32(long rhs) noexcept
{
    auto r = socket_event::none;

    r |= (rhs & FD_READ) ? socket_event::read : socket_event::none;
    r |= (rhs & FD_WRITE) ? socket_event::write : socket_event::none;
    r |= (rhs & FD_CLOSE) ? socket_event::close : socket_event::none;
    r |= (rhs & FD_CONNECT) ? socket_event::connect : socket_event::none;
    r |= (rhs & FD_ACCEPT) ? socket_event::accept : socket_event::none;
    r |= (rhs & FD_OOB) ? socket_event::out_of_band : socket_event::none;
    r |= (rhs & FD_QOS) ? socket_event::qos : socket_event::none;
    r |= (rhs & FD_GROUP_QOS) ? socket_event::group_qos : socket_event::none;
    r |= (rhs & FD_ADDRESS_LIST_CHANGE) ? socket_event::address_list_change : socket_event::none;
    r |= (rhs & FD_ROUTING_INTERFACE_CHANGE) ? socket_event::routing_interface_changed : socket_event::none;

    return r;
}

[[nodiscard]] constexpr socket_error socket_error_from_win32(int rhs) noexcept
{
    switch (rhs) {
    case 0: return socket_error::success;
    case WSAEAFNOSUPPORT: return socket_error::af_not_supported;
    case WSAECONNREFUSED: return socket_error::connection_refused;
    case WSAENETUNREACH: return socket_error::network_unreachable;
    case WSAENOBUFS: return socket_error::no_buffers;
    case WSAETIMEDOUT: return socket_error::timeout;
    case WSAENETDOWN: return socket_error::network_down;
    case WSAECONNRESET: return socket_error::connection_reset;
    case WSAECONNABORTED: return socket_error::connection_aborted;
    default: hi_no_default();
    }
}

[[nodiscard]] constexpr socket_events socket_events_from_win32(WSANETWORKEVENTS const& rhs) noexcept
{
    auto r = socket_events{};
    r.events = socket_event_from_win32(rhs.lNetworkEvents);
    r.errors[bit(socket_event::read)] = socket_error_from_win32(rhs.iErrorCode[FD_READ_BIT]);
    r.errors[bit(socket_event::write)] = socket_error_from_win32(rhs.iErrorCode[FD_WRITE_BIT]);
    r.errors[bit(socket_event::close)] = socket_error_from_win32(rhs.iErrorCode[FD_CLOSE_BIT]);
    r.errors[bit(socket_event::connect)] = socket_error_from_win32(rhs.iErrorCode[FD_CONNECT_BIT]);
    r.errors[bit(socket_event::accept)] = socket_error_from_win32(rhs.iErrorCode[FD_ACCEPT_BIT]);
    r.errors[bit(socket_event::out_of_band)] = socket_error_from_win32(rhs.iErrorCode[FD_OOB_BIT]);
    r.errors[bit(socket_event::qos)] = socket_error_from_win32(rhs.iErrorCode[FD_QOS_BIT]);
    r.errors[bit(socket_event::group_qos)] = socket_error_from_win32(rhs.iErrorCode[FD_GROUP_QOS_BIT]);
    r.errors[bit(socket_event::address_list_change)] = socket_error_from_win32(rhs.iErrorCode[FD_ADDRESS_LIST_CHANGE_BIT]);
    r.errors[bit(socket_event::routing_interface_changed)] =
        socket_error_from_win32(rhs.iErrorCode[FD_ROUTING_INTERFACE_CHANGE_BIT]);

    return r;
}

} // namespace hi::inline v1
