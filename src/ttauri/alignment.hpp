// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/// @file

#pragma once

#include "assert.hpp"

namespace tt::inline v1 {

/** Vertical alignment.
 */
enum class vertical_alignment {
    /** Align to the top.
     */
    top,

    /** Align to the vertical-middle.
     */
    middle,

    /** Align to the bottom.
     */
    bottom
};

/** Horizontal alignment.
 */
enum class horizontal_alignment {
    /** Align to the left.
     */
    left,

    /** Align to the horizontal-center.
     */
    center,

    /** Align to the right.
     */
    right
};

/** Vertical and horizontal alignment.
 */
enum class alignment {
    /** Align to the top and left.
     */
    top_left,

    /** Align to the top and horizontal-center.
     */
    top_center,

    /** Align to the top and right.
     */
    top_right,

    /** Align to the vertical-middle and left.
     */
    middle_left,

    /** Align to the vertical-middle and horizontal-center.
     */
    middle_center,

    /** Align to the vertical-middle and right.
     */
    middle_right,

    /** Align to the bottom and left.
     */
    bottom_left,

    /** Align to the bottom and horizontal-center.
     */
    bottom_center,

    /** Align to the bottom and right.
     */
    bottom_right
};

/** Combine vertical and horizontal alignment.
 *
 * @param lhs A vertical alignment.
 * @param rhs A horizontal alignment.
 * @return A combined vertical and horizontal alignment.
 */
constexpr alignment operator|(vertical_alignment lhs, horizontal_alignment rhs) noexcept
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

/** Combine vertical and horizontal alignment.
 *
 * @param lhs A horizontal alignment.
 * @param rhs A vertical alignment.
 * @return A combined vertical and horizontal alignment.
 */
constexpr alignment operator|(horizontal_alignment lhs, vertical_alignment rhs) noexcept
{
    return rhs | lhs;
}

/** Check if the horizontal alignments are equal.
 *
 * @param lhs A combined vertical and horizontal alignment.
 * @param rhs A horizontal alignment.
 * @return True when the horizontal alignment of both `lhs` and `rhs` are equal.
 */
constexpr bool operator==(alignment lhs, horizontal_alignment rhs) noexcept
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

/** Check if the vertical alignments are equal.
 *
 * @param lhs A combined vertical and horizontal alignment.
 * @param rhs A vertical alignment.
 * @return True when the vertical alignment of both `lhs` and `rhs` are equal.
 */
constexpr bool operator==(alignment lhs, vertical_alignment rhs) noexcept
{
    switch (rhs) {
    case vertical_alignment::top:
        return lhs == alignment::top_left || lhs == alignment::top_center || lhs == alignment::top_right;

    case vertical_alignment::middle:
        return lhs == alignment::middle_left || lhs == alignment::middle_center || lhs == alignment::middle_right;

    case vertical_alignment::bottom:
        return lhs == alignment::bottom_left || lhs == alignment::bottom_center || lhs == alignment::bottom_right;

    default: tt_no_default();
    }
}

} // namespace tt::inline v1
