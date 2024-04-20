

#pragma once

#include "enum_metadata.hpp"

namespace hi { inline namespace v1 {

/** The device type this application is running on.
 *
 * The underlying value can be used as the base pixel density
 * after bitwise-and with 0xf8.
 */
enum class device_type : unsigned char {
    desktop = 120, // low density display
    server = 121,
    watch = 160, // medium density display
    phone = 161,
    tablet = 162,
    game_console = 208, // (1.33 * medium density display) & 0xf8
    television = 209,
};

// clang-format off
constexpr auto device_type_metadata = enum_metadata{
    device_type::desktop, "desktop",
    device_type::server, "server",
    device_type::watch, "watch",
    device_type::phone, "phone",
    device_type::tablet, "tablet",
    device_type::game_console, "game console",
    device_type::television, "television"
};
// clang-format on

}}

// XXX #617 MSVC bug does not handle partial specialization in modules.
hi_export template<>
struct std::formatter<hi::device_type, char> : std::formatter<std::string_view, char> {
    auto format(hi::device_type const& t, auto& fc) const
    {
        return std::formatter<std::string_view, char>::format(hi::device_type_metadata[t], fc);
    }
};

