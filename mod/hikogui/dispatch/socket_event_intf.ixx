// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <bit>

export module hikogui_dispatch_socket_event : intf;
import hikogui_utility;

export namespace hi::inline v1 {

enum class socket_event : uint16_t {
    none = 0,
    read = 0x0001,
    write = 0x0002,
    close = 0x0004,
    connect = 0x0008,
    accept = 0x0010,
    out_of_band = 0x0020,
    qos = 0x0040,
    group_qos = 0x0080,
    address_list_change = 0x0100,
    routing_interface_changed = 0x0200
};

[[nodiscard]] constexpr socket_event operator|(socket_event const& lhs, socket_event const& rhs) noexcept
{
    return static_cast<socket_event>(std::to_underlying(lhs) | std::to_underlying(rhs));
}

[[nodiscard]] constexpr socket_event operator&(socket_event const& lhs, socket_event const& rhs) noexcept
{
    return static_cast<socket_event>(std::to_underlying(lhs) & std::to_underlying(rhs));
}

constexpr socket_event& operator|=(socket_event& lhs, socket_event const& rhs) noexcept
{
    return lhs = lhs | rhs;
}

[[nodiscard]] constexpr bool to_bool(socket_event const& rhs) noexcept
{
    return to_bool(std::to_underlying(rhs));
}

/** Get the bit index of the single bit of the socket_event mask.
 */
[[nodiscard]] constexpr size_t bit(socket_event const& rhs) noexcept
{
    hi_assert(std::popcount(std::to_underlying(rhs)) == 1);
    return std::countr_zero(std::to_underlying(rhs));
}

enum class socket_error : uint8_t {
    success = 0,
    af_not_supported,
    connection_refused,
    network_unreachable,
    no_buffers,
    timeout,
    network_down,
    connection_reset,
    connection_aborted
};

constexpr size_t socket_event_max = 10;

class socket_events {
public:
    socket_event events;
    std::array<socket_error, socket_event_max> errors;

    constexpr socket_events() noexcept : events(socket_event::none), errors{} {}
};

} // namespace hi::inline v1
