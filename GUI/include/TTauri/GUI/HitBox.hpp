// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include <limits>

namespace TTauri::GUI {

namespace Widgets {
class Widget;
}

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

    Widgets::Widget *widget;
    float depth;
    Type type;

    HitBox(Widgets::Widget *widget=nullptr, float depth=std::numeric_limits<float>::max(), Type type=Type::Outside) noexcept :
        widget(widget), depth(depth), type(type) {}


    friend bool operator==(HitBox const &lhs, HitBox const &rhs) noexcept {
        return lhs.widget == rhs.widget && lhs.depth == rhs.depth && lhs.type == rhs.type;
    }

    friend bool operator!=(HitBox const &lhs, HitBox const &rhs) noexcept {
        return !(lhs == rhs);
    }

    friend bool operator<(HitBox const &lhs, HitBox const &rhs) noexcept {
        if ((lhs.widget == nullptr) == (rhs.widget == nullptr)) {
            if (lhs.depth == rhs.depth) {
                return static_cast<int>(lhs.type) < static_cast<int>(rhs.type);
            } else {
                // We actually want to check if a hitbox is above another hitbox;
                // which means the inverse of depth.
                return lhs.depth > rhs.depth;
            }
        } else {
            return lhs.widget == nullptr;
        }
    }

    friend bool operator>(HitBox const &lhs, HitBox const &rhs) noexcept {
        return rhs < lhs;
    }

    friend bool operator<=(HitBox const &lhs, HitBox const &rhs) noexcept {
        return !(lhs > rhs);
    }

    friend bool operator>=(HitBox const &lhs, HitBox const &rhs) noexcept {
        return !(lhs < rhs);
    }

};

}