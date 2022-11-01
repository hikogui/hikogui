// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../geometry/point.hpp"
#include "../assert.hpp"
#include <limits>
#include <cstdint>
#include <compare>

namespace hi::inline v1 {
class widget;

enum class hitbox_type : uint8_t {
    outside,
    _default,
    button,
    scroll_bar,
    text_edit,
    move_area,
    bottom_resize_border,
    top_resize_border,
    left_resize_border,
    right_resize_border,
    bottom_left_resize_corner,
    bottom_right_resize_corner,
    top_left_resize_corner,
    top_right_resize_corner,
    application_icon
};

class hitbox {
public:
    hitbox_type type;
    widget const *widget;

    constexpr hitbox(hitbox const&) noexcept = default;
    constexpr hitbox(hitbox&&) noexcept = default;
    constexpr hitbox& operator=(hitbox const&) noexcept = default;
    constexpr hitbox& operator=(hitbox&&) noexcept = default;

    constexpr hitbox() noexcept : widget(nullptr), _elevation(-std::numeric_limits<float>::max()), type(hitbox_type::outside) {}

    constexpr hitbox(
        hi::widget const *widget,
        float elevation = -std::numeric_limits<float>::max(),
        hitbox_type type = hitbox_type::_default) noexcept :
        widget(widget), _elevation(elevation), type(type)
    {
    }

    constexpr hitbox(hi::widget const *widget, point3 position, hitbox_type type = hitbox_type::_default) noexcept :
        widget(widget), _elevation(-position.z()), type(type)
    {
    }

    [[nodiscard]] constexpr friend std::strong_ordering operator<=>(hitbox const& lhs, hitbox const& rhs) noexcept
    {
        if ((lhs.widget == nullptr) == (rhs.widget == nullptr)) {
            // Either both are widgets, or both are not widgets.
            hilet elevation_ordering = lhs._elevation <=> rhs._elevation;
            if (elevation_ordering == std::partial_ordering::equivalent) {
                return to_underlying(lhs.type) <=> to_underlying(rhs.type);
            } else if (elevation_ordering == std::partial_ordering::less) {
                return std::strong_ordering::less;
            } else if (elevation_ordering == std::partial_ordering::greater) {
                return std::strong_ordering::greater;
            } else {
                hi_no_default();
            }
        } else if (lhs.widget == nullptr) {
            // If lhs is not a widget than it is less.
            return std::strong_ordering::less;
        } else {
            // Otherwise the lhs is greater.
            return std::strong_ordering::greater;
        }
    }

private:
    float _elevation;
};

} // namespace hi::inline v1
