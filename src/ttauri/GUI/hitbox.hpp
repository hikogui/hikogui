// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../geometry/point.hpp"
#include "../assert.hpp"
#include <limits>
#include <cstdint>
#include <compare>

namespace tt::inline v1 {
class widget;

class hitbox {
public:
    enum class Type : uint8_t {
        Outside,
        Default,
        Button,
        TextEdit,
        MoveArea,
        BottomResizeBorder,
        TopResizeBorder,
        LeftResizeBorder,
        RightResizeBorder,
        BottomLeftResizeCorner,
        BottomRightResizeCorner,
        TopLeftResizeCorner,
        TopRightResizeCorner,
        ApplicationIcon
    };

    Type type;
    widget const *widget;

    constexpr hitbox(hitbox const &) noexcept = default;
    constexpr hitbox(hitbox &&) noexcept = default;
    constexpr hitbox &operator=(hitbox const &) noexcept = default;
    constexpr hitbox &operator=(hitbox &&) noexcept = default;

    constexpr hitbox() noexcept : widget(nullptr), _elevation(-std::numeric_limits<float>::max()), type(Type::Outside) {}

    constexpr hitbox(
        tt::widget const *widget,
        float elevation = -std::numeric_limits<float>::max(),
        Type type = Type::Default) noexcept :
        widget(widget), _elevation(elevation), type(type)
    {
    }

    constexpr hitbox(tt::widget const *widget, point3 position, Type type = Type::Default) noexcept :
        widget(widget), _elevation(-position.z()), type(type)
    {
    }

    [[nodiscard]] constexpr friend std::strong_ordering operator<=>(hitbox const &lhs, hitbox const &rhs) noexcept
    {
        if ((lhs.widget == nullptr) == (rhs.widget == nullptr)) {
            // Either both are widgets, or both are not widgets.
            ttlet elevation_cmp = lhs._elevation <=> rhs._elevation;
            if (elevation_cmp == 0) {
                return static_cast<int>(lhs.type) <=> static_cast<int>(rhs.type);
            } else if (elevation_cmp < 0) {
                return std::strong_ordering::less;
            } else if (elevation_cmp > 0) {
                return std::strong_ordering::greater;
            } else {
                tt_no_default();
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

} // namespace tt::inline v1
