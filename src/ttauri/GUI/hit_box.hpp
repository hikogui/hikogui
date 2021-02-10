// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <limits>
#include <cstdint>
#include <memory>

namespace tt {

class widget;

struct hit_box {
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

    std::weak_ptr<widget const> widget;
    float elevation;
    Type type;

    hit_box() noexcept : widget({}), elevation(-std::numeric_limits<float>::max()), type(Type::Outside)
    {
    }

    hit_box(std::weak_ptr<tt::widget const> widget, float elevation = -std::numeric_limits<float>::max(), Type type = Type::Default) noexcept
        :
        widget(widget), elevation(elevation), type(type)
    {
    }

    friend bool operator<(hit_box const &lhs, hit_box const &rhs) noexcept {
        if (lhs.widget.expired() == rhs.widget.expired()) {
            if (lhs.elevation == rhs.elevation) {
                return static_cast<int>(lhs.type) < static_cast<int>(rhs.type);
            } else {
                // We actually want to check if a hitbox is above another hitbox;
                // which means the inverse of depth.
                return lhs.elevation < rhs.elevation;
            }
        } else {
            return lhs.widget.expired();
        }
    }

};

}
