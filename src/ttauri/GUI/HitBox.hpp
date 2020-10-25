// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include <limits>
#include <cstdint>
#include <memory>

namespace tt {

class Widget;

struct HitBox {
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

    std::weak_ptr<Widget const> widget;
    float elevation;
    Type type;

    HitBox() noexcept : widget({}), elevation(-std::numeric_limits<float>::max()), type(Type::Outside)
    {
    }

    HitBox(std::weak_ptr<Widget const> widget, float elevation = -std::numeric_limits<float>::max(), Type type = Type::Default) noexcept
        :
        widget(widget), elevation(elevation), type(type)
    {
    }


    //friend bool operator==(HitBox const &lhs, HitBox const &rhs) noexcept {
    //    return lhs.widget == rhs.widget && lhs.elevation == rhs.elevation && lhs.type == rhs.type;
    //}

    //friend bool operator!=(HitBox const &lhs, HitBox const &rhs) noexcept {
    //    return !(lhs == rhs);
    //}

    friend bool operator<(HitBox const &lhs, HitBox const &rhs) noexcept {
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

    //friend bool operator>(HitBox const &lhs, HitBox const &rhs) noexcept {
    //    return rhs < lhs;
    //}

    //friend bool operator<=(HitBox const &lhs, HitBox const &rhs) noexcept {
    //    return !(lhs > rhs);
    //}

    //friend bool operator>=(HitBox const &lhs, HitBox const &rhs) noexcept {
    //    return !(lhs < rhs);
    //}

};

}
