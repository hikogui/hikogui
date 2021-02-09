// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "required.hpp"
#include <compare>

namespace tt {

enum class arrangement : bool { column, row };

enum class vertical_alignment { top, middle, bottom };

enum class horizontal_alignment { left, center, right };

enum class alignment {
    top_left,
    top_center,
    top_right,
    middle_left,
    middle_center,
    middle_right,
    bottom_left,
    bottom_center,
    bottom_right
};

inline alignment operator|(vertical_alignment lhs, horizontal_alignment rhs) noexcept
{
    switch (lhs) {
    case vertical_alignment::top:
        switch (rhs) {
        case horizontal_alignment::left: return alignment::top_left;
        case horizontal_alignment::center: return alignment::top_center;
        case horizontal_alignment::right: return alignment::top_right;
        default: tt_no_default();
        }
    case vertical_alignment::middle:
        switch (rhs) {
        case horizontal_alignment::left: return alignment::middle_left;
        case horizontal_alignment::center: return alignment::middle_center;
        case horizontal_alignment::right: return alignment::middle_right;
        default: tt_no_default();
        }
    case vertical_alignment::bottom:
        switch (rhs) {
        case horizontal_alignment::left: return alignment::bottom_left;
        case horizontal_alignment::center: return alignment::bottom_center;
        case horizontal_alignment::right: return alignment::bottom_right;
        default: tt_no_default();
        }
    default: tt_no_default();
    }
}

inline alignment operator|(horizontal_alignment lhs, vertical_alignment rhs) noexcept
{
    return rhs | lhs;
}

inline bool operator==(alignment lhs, horizontal_alignment rhs) noexcept
{
    switch (rhs) {
    case horizontal_alignment::left:
        return lhs == alignment::top_left || lhs == alignment::middle_left || lhs == alignment::bottom_left;

    case horizontal_alignment::center:
        return lhs == alignment::top_center || lhs == alignment::middle_center || lhs == alignment::bottom_center;

    case horizontal_alignment::right:
        return lhs == alignment::top_right || lhs == alignment::middle_right || lhs == alignment::bottom_right;

    default: tt_no_default();
    }
}

inline bool operator==(alignment lhs, vertical_alignment rhs) noexcept
{
    switch (rhs) {
    case vertical_alignment::top: return lhs == alignment::top_left || lhs == alignment::top_center || lhs == alignment::top_right;

    case vertical_alignment::middle:
        return lhs == alignment::middle_left || lhs == alignment::middle_center || lhs == alignment::middle_right;

    case vertical_alignment::bottom:
        return lhs == alignment::bottom_left || lhs == alignment::bottom_center || lhs == alignment::bottom_right;

    default: tt_no_default();
    }
}

inline bool operator!=(alignment lhs, horizontal_alignment rhs) noexcept
{
    return !(lhs == rhs);
}

inline bool operator!=(alignment lhs, vertical_alignment rhs) noexcept
{
    return !(lhs == rhs);
}

class relative_base_line {
    vertical_alignment alignment;
    float offset;
    float priority;

public:
    /* Construct a base-line.
     * @param alignment Start position of the base line.
     * @param offset Number of pt above the start position.
     * @param priority Higher values will have priority over lower values.
     */
    constexpr relative_base_line(vertical_alignment alignment, float offset = 0.0f, float priority = 100.0f) noexcept :
        alignment(alignment), offset(offset), priority(priority)
    {
    }

    /** Constructs a low-priority base line in the middle.
     */
    constexpr relative_base_line() noexcept : relative_base_line(vertical_alignment::middle, 0.0f, 0.0f) {}

    /** Get a base-line position.
     */
    constexpr float position(float bottom, float top) const noexcept
    {
        switch (alignment) {
        case vertical_alignment::bottom: return bottom + offset;
        case vertical_alignment::top: return top + offset;
        case vertical_alignment::middle: return (bottom + top) * 0.5f + offset;
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
