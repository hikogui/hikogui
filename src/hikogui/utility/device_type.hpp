

#pragma once

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

}}


