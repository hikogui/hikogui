// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../cast.hpp"
#include "../assert.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <bit>

namespace hi::inline v1 {

enum class network_event : uint16_t {
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

[[nodiscard]] constexpr network_event operator|(network_event const& lhs, network_event const& rhs) noexcept
{
    return static_cast<network_event>(to_underlying(lhs) | to_underlying(rhs));
}

[[nodiscard]] constexpr network_event operator&(network_event const& lhs, network_event const& rhs) noexcept
{
    return static_cast<network_event>(to_underlying(lhs) & to_underlying(rhs));
}

constexpr network_event& operator|=(network_event& lhs, network_event const& rhs) noexcept
{
    return lhs = lhs | rhs;
}

[[nodiscard]] constexpr bool to_bool(network_event const& rhs) noexcept
{
    return to_bool(to_underlying(rhs));
}

/** Get the bit index of the single bit of the network_event mask.
 */
[[nodiscard]] constexpr size_t bit(network_event const& rhs) noexcept
{
    hi_assert(std::popcount(to_underlying(rhs)) == 1);
    return std::countr_zero(to_underlying(rhs));
}

enum class network_error : uint8_t {
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

constexpr static size_t network_event_max = 10;

class network_events {
public:
    network_event events;
    std::array<network_error, network_event_max> errors;

    constexpr network_events() noexcept : events(network_event::none), errors{} {}
};

} // namespace hi::inline v1
