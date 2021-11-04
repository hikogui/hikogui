// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <limits>
#include <cstdint>
#include "../geometry/point.hpp"

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

    friend bool operator<(hitbox const &lhs, hitbox const &rhs) noexcept
    {
        if ((lhs.widget == nullptr) == (rhs.widget == nullptr)) {
            if (lhs._elevation == rhs._elevation) {
                return static_cast<int>(lhs.type) < static_cast<int>(rhs.type);
            } else {
                // We actually want to check if a hitbox is above another hitbox;
                // which means the inverse of depth.
                return lhs._elevation < rhs._elevation;
            }
        } else {
            return lhs.widget == nullptr;
        }
    }

private:
    float _elevation;
};

} // namespace tt::inline v1
