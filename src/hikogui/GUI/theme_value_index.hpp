// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file GUI/theme_value_index.hpp An index into the theme-value
 */

#pragma once

#include "widget_intf.hpp"

namespace hi { inline namespace v1 {

class theme_value_index {
public:
    /** The theme value index is 6-bits.
     *
     * The theme value index is a bit-field as follows.
     * - [1:0] :disabled = '00', :enabled = '01', :focus = '10', :pressed = '11'
     * - [2:2] :inactive = '0', :active = '1'
     * - [3:3] :off = '0', :on = '1'
     * - [5:4] depth % 4
     */
    using value_type = uint8_t;

    constexpr static size_t array_size = 0x40_uz;

    constexpr theme_value_index() noexcept = default;
    constexpr theme_value_index(theme_value_index const&) noexcept = default;
    constexpr theme_value_index(theme_value_index&&) noexcept = default;
    constexpr theme_value_index& operator=(theme_value_index const&) noexcept = default;
    constexpr theme_value_index& operator=(theme_value_index&&) noexcept = default;

    constexpr theme_value_index(intrinsic_t, value_type value) noexcept : _v(value) {}

    theme_value_index(widget_intf const& widget) noexcept
    {
        // [1:0] : disabled = '00', : enabled = '01', : focus = '10', : pressed = '11'
        // [2:2] : inactive = '0', : active = '1'
        // [3:3] : off = '0', : on = '1'
        // [5:4] depth % 4
        auto tmp = value_type{};
        tmp |= widget.semantic_layer % 4;
        tmp <<= 2;
        tmp |= wide_cast<value_type>(*widget.on);
        tmp <<= 1;
        tmp |= wide_cast<value_type>(*widget.active);
        tmp <<= 1;
        tmp |= [&] {
            if (*widget.mode <= widget_mode::disabled) {
                return 0;
            } else if (*widget.clicked) {
                return 3;
            } else if (*widget.focus) {
                return 2;
            } else {
                return 1;
            }
        }();

        _v = tmp;
    }

    constexpr value_type& intrinsic() noexcept
    {
        return _v;
    }

    constexpr value_type const& intrinsic() const noexcept
    {
        return _v;
    }

private:
    value_type _v;
};

}} // namespace hi::v1
