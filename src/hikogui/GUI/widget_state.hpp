


#pragma once

namespace hi {
inline namespace v1 {

class enum widget_state : uint8_t {
    /** Widget's mode is disabled.
     */
    enabled = 0b000001,

    /** The window is inactive.
     */
    active  = 0b000010,

    on      = 0b000100,
    focus   = 0b001000,
    hover   = 0b010000,
    pressed = 0b100000,
};

}}

