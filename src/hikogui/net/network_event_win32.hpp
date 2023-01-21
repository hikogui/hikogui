// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "network_event.hpp"
#include "../utility/module.hpp"

namespace hi::inline v1 {

[[nodiscard]] constexpr network_event network_event_from_win32(long rhs) noexcept
{
    auto r = network_event::none;

    r |= (rhs & FD_READ) ? network_event::read : network_event::none;
    r |= (rhs & FD_WRITE) ? network_event::write : network_event::none;
    r |= (rhs & FD_CLOSE) ? network_event::close : network_event::none;
    r |= (rhs & FD_CONNECT) ? network_event::connect : network_event::none;
    r |= (rhs & FD_ACCEPT) ? network_event::accept : network_event::none;
    r |= (rhs & FD_OOB) ? network_event::out_of_band : network_event::none;
    r |= (rhs & FD_QOS) ? network_event::qos : network_event::none;
    r |= (rhs & FD_GROUP_QOS) ? network_event::group_qos : network_event::none;
    r |= (rhs & FD_ADDRESS_LIST_CHANGE) ? network_event::address_list_change : network_event::none;
    r |= (rhs & FD_ROUTING_INTERFACE_CHANGE) ? network_event::routing_interface_changed : network_event::none;

    return r;
}

[[nodiscard]] constexpr network_error network_error_from_win32(int rhs) noexcept
{
    switch (rhs) {
    case 0: return network_error::success;
    case WSAEAFNOSUPPORT: return network_error::af_not_supported;
    case WSAECONNREFUSED: return network_error::connection_refused;
    case WSAENETUNREACH: return network_error::network_unreachable;
    case WSAENOBUFS: return network_error::no_buffers;
    case WSAETIMEDOUT: return network_error::timeout;
    case WSAENETDOWN: return network_error::network_down;
    case WSAECONNRESET: return network_error::connection_reset;
    case WSAECONNABORTED: return network_error::connection_aborted;
    default: hi_no_default();
    }
}

[[nodiscard]] constexpr network_events network_events_from_win32(WSANETWORKEVENTS const& rhs) noexcept
{
    auto r = network_events{};
    r.events = network_event_from_win32(rhs.lNetworkEvents);
    r.errors[bit(network_event::read)] = network_error_from_win32(rhs.iErrorCode[FD_READ_BIT]);
    r.errors[bit(network_event::write)] = network_error_from_win32(rhs.iErrorCode[FD_WRITE_BIT]);
    r.errors[bit(network_event::close)] = network_error_from_win32(rhs.iErrorCode[FD_CLOSE_BIT]);
    r.errors[bit(network_event::connect)] = network_error_from_win32(rhs.iErrorCode[FD_CONNECT_BIT]);
    r.errors[bit(network_event::accept)] = network_error_from_win32(rhs.iErrorCode[FD_ACCEPT_BIT]);
    r.errors[bit(network_event::out_of_band)] = network_error_from_win32(rhs.iErrorCode[FD_OOB_BIT]);
    r.errors[bit(network_event::qos)] = network_error_from_win32(rhs.iErrorCode[FD_QOS_BIT]);
    r.errors[bit(network_event::group_qos)] = network_error_from_win32(rhs.iErrorCode[FD_GROUP_QOS_BIT]);
    r.errors[bit(network_event::address_list_change)] = network_error_from_win32(rhs.iErrorCode[FD_ADDRESS_LIST_CHANGE_BIT]);
    r.errors[bit(network_event::routing_interface_changed)] =
        network_error_from_win32(rhs.iErrorCode[FD_ROUTING_INTERFACE_CHANGE_BIT]);

    return r;
}

} // namespace hi::inline v1
