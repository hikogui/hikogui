// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/required.hpp"

namespace TTauri {

enum class VerticalAlignment {
    Top,
    Middle,
    Bottom
};

enum class HorizontalAlignment {
    Left,
    Center,
    Right
};

enum class Alignment {
    TopLeft,
    TopCenter,
    TopRight,
    MiddleLeft,
    MiddleCenter,
    MiddleRight,
    BottomLeft,
    BottomCenter,
    BottomRight
};

enum class LineJoinStyle {
    Bevel,
    Miter,
    Rounded
};

inline Alignment operator|(VerticalAlignment lhs, HorizontalAlignment rhs) noexcept
{
    switch (lhs) {
    case VerticalAlignment::Top:
        switch (rhs) {
        case HorizontalAlignment::Left: return Alignment::TopLeft;
        case HorizontalAlignment::Center: return Alignment::TopCenter;
        case HorizontalAlignment::Right: return Alignment::TopRight;
        default: no_default;
        }
    case VerticalAlignment::Middle:
        switch (rhs) {
        case HorizontalAlignment::Left: return Alignment::MiddleLeft;
        case HorizontalAlignment::Center: return Alignment::MiddleCenter;
        case HorizontalAlignment::Right: return Alignment::MiddleRight;
        default: no_default;
        }
    case VerticalAlignment::Bottom:
        switch (rhs) {
        case HorizontalAlignment::Left: return Alignment::BottomLeft;
        case HorizontalAlignment::Center: return Alignment::BottomCenter;
        case HorizontalAlignment::Right: return Alignment::BottomRight;
        default: no_default;
        }
    default: no_default;
    }
}

inline Alignment operator|(HorizontalAlignment lhs, VerticalAlignment rhs) noexcept
{
    return rhs | lhs;
}

inline bool operator==(Alignment lhs, HorizontalAlignment rhs) noexcept
{
    switch (rhs) {
    case HorizontalAlignment::Left:
        return
            lhs == Alignment::TopLeft ||
            lhs == Alignment::MiddleLeft ||
            lhs == Alignment::BottomLeft;

    case HorizontalAlignment::Center:
        return
            lhs == Alignment::TopCenter ||
            lhs == Alignment::MiddleCenter ||
            lhs == Alignment::BottomCenter;

    case HorizontalAlignment::Right:
        return
            lhs == Alignment::TopRight ||
            lhs == Alignment::MiddleRight ||
            lhs == Alignment::BottomRight;

    default: no_default;
    }
}

inline bool operator==(Alignment lhs, VerticalAlignment rhs) noexcept
{
    switch (rhs) {
    case VerticalAlignment::Top:
        return
            lhs == Alignment::TopLeft ||
            lhs == Alignment::TopCenter ||
            lhs == Alignment::TopRight;

    case VerticalAlignment::Middle:
        return
            lhs == Alignment::MiddleLeft ||
            lhs == Alignment::MiddleCenter ||
            lhs == Alignment::MiddleRight;

    case VerticalAlignment::Bottom:
        return
            lhs == Alignment::BottomLeft ||
            lhs == Alignment::BottomCenter ||
            lhs == Alignment::BottomRight;

    default: no_default;
    }
}

inline bool operator!=(Alignment lhs, HorizontalAlignment rhs) noexcept
{
    return !(lhs == rhs);
}

inline bool operator!=(Alignment lhs, VerticalAlignment rhs) noexcept
{
    return !(lhs == rhs);
}

}