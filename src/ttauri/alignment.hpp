// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "required.hpp"
#include <compare>

namespace tt {

enum class arrangement : bool { column, row };

enum class VerticalAlignment { Top, Middle, Bottom };

enum class HorizontalAlignment { Left, Center, Right };

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

enum class LineJoinStyle { Bevel, Miter, Rounded };



inline Alignment operator|(VerticalAlignment lhs, HorizontalAlignment rhs) noexcept
{
    switch (lhs) {
    case VerticalAlignment::Top:
        switch (rhs) {
        case HorizontalAlignment::Left: return Alignment::TopLeft;
        case HorizontalAlignment::Center: return Alignment::TopCenter;
        case HorizontalAlignment::Right: return Alignment::TopRight;
        default: tt_no_default();
        }
    case VerticalAlignment::Middle:
        switch (rhs) {
        case HorizontalAlignment::Left: return Alignment::MiddleLeft;
        case HorizontalAlignment::Center: return Alignment::MiddleCenter;
        case HorizontalAlignment::Right: return Alignment::MiddleRight;
        default: tt_no_default();
        }
    case VerticalAlignment::Bottom:
        switch (rhs) {
        case HorizontalAlignment::Left: return Alignment::BottomLeft;
        case HorizontalAlignment::Center: return Alignment::BottomCenter;
        case HorizontalAlignment::Right: return Alignment::BottomRight;
        default: tt_no_default();
        }
    default: tt_no_default();
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
        return lhs == Alignment::TopLeft || lhs == Alignment::MiddleLeft || lhs == Alignment::BottomLeft;

    case HorizontalAlignment::Center:
        return lhs == Alignment::TopCenter || lhs == Alignment::MiddleCenter || lhs == Alignment::BottomCenter;

    case HorizontalAlignment::Right:
        return lhs == Alignment::TopRight || lhs == Alignment::MiddleRight || lhs == Alignment::BottomRight;

    default: tt_no_default();
    }
}

inline bool operator==(Alignment lhs, VerticalAlignment rhs) noexcept
{
    switch (rhs) {
    case VerticalAlignment::Top: return lhs == Alignment::TopLeft || lhs == Alignment::TopCenter || lhs == Alignment::TopRight;

    case VerticalAlignment::Middle:
        return lhs == Alignment::MiddleLeft || lhs == Alignment::MiddleCenter || lhs == Alignment::MiddleRight;

    case VerticalAlignment::Bottom:
        return lhs == Alignment::BottomLeft || lhs == Alignment::BottomCenter || lhs == Alignment::BottomRight;

    default: tt_no_default();
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

class relative_base_line {
    VerticalAlignment alignment;
    float offset;
    float priority;

public:
    /* Construct a base-line.
     * @param alignment Start position of the base line.
     * @param offset Number of pt above the start position.
     * @param priority Higher values will have priority over lower values.
     */
    constexpr relative_base_line(VerticalAlignment alignment, float offset = 0.0f, float priority = 100.0f) noexcept :
        alignment(alignment), offset(offset), priority(priority)
    {
    }

    /** Constructs a low-priority base line in the middle.
     */
    constexpr relative_base_line() noexcept : relative_base_line(VerticalAlignment::Middle, 0.0f, 0.0f) {}

    /** Get a base-line position.
     */
    constexpr float position(float bottom, float top) const noexcept
    {
        switch (alignment) {
        case VerticalAlignment::Bottom: return bottom + offset;
        case VerticalAlignment::Top: return top + offset;
        case VerticalAlignment::Middle: return (bottom + top) * 0.5f + offset;
        }
        tt_no_default();
    }

    [[nodiscard]] auto operator==(relative_base_line const &rhs) const noexcept
    {
        return this->priority == rhs.priority;
    }

    [[nodiscard]] auto operator<=>(relative_base_line const &rhs) const noexcept
    {
        return this->priority <=> rhs.priority;
    }
};

} // namespace tt
