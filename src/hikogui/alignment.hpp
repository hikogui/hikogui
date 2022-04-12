// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file alignment.hpp
 */

#pragma once

#include "assert.hpp"
#include "cast.hpp"

namespace tt::inline v1 {

/** Vertical alignment.
 */
enum class vertical_alignment : uint8_t {
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

enum class horizontal_alignment : uint8_t {
    /** Align the text naturally based on the writing direction of each paragraph.
     *
     * This will act as flush_left if the paragraph is in left-to-right direction,
     * and as flush_right if the paragraph is in right-to-left direction.
     */
    flush,

    /** Align the text to the left side
     *
     * The text will be flush-left independent of the writing direction.
     */
    left,

    /** Align the text in the center.
     *
     * Since the text is centered, the writing direction is unimportant.
     */
    center,

    /** Stretch the text to be flush to both sides.
     *
     * Since the text is flush on both sides, the writing direction is unimportant.
     */
    justified,

    /** Align the text to the right side
     *
     * The text will be flush-left independent of the writing direction.
     */
    right,
};

class alignment {
public:
    constexpr alignment() noexcept : _value(0) {}
    constexpr alignment(alignment const &) noexcept = default;
    constexpr alignment(alignment &&) noexcept = default;
    constexpr alignment &operator=(alignment const &) noexcept = default;
    constexpr alignment &operator=(alignment &&) noexcept = default;

    constexpr explicit alignment(uint8_t value) noexcept : _value(value) {}

    constexpr alignment(horizontal_alignment t, vertical_alignment v) noexcept :
        _value((to_underlying(v) << 4) | to_underlying(t))
    {
        tt_axiom(to_underlying(v) <= 0xf);
        tt_axiom(to_underlying(t) <= 0xf);
    }

    constexpr alignment(vertical_alignment v, horizontal_alignment t) noexcept :
        _value((to_underlying(v) << 4) | to_underlying(t))
    {
        tt_axiom(to_underlying(v) <= 0xf);
        tt_axiom(to_underlying(t) <= 0xf);
    }

    [[nodiscard]] static constexpr alignment top_flush() noexcept
    {
        return {horizontal_alignment::flush, vertical_alignment::top};
    }

    [[nodiscard]] static constexpr alignment top_left() noexcept
    {
        return {horizontal_alignment::left, vertical_alignment::top};
    }

    [[nodiscard]] static constexpr alignment top_center() noexcept
    {
        return {horizontal_alignment::center, vertical_alignment::top};
    }

    [[nodiscard]] static constexpr alignment top_justified() noexcept
    {
        return {horizontal_alignment::justified, vertical_alignment::top};
    }

    [[nodiscard]] static constexpr alignment top_right() noexcept
    {
        return {horizontal_alignment::right, vertical_alignment::top};
    }

    [[nodiscard]] static constexpr alignment middle_flush() noexcept
    {
        return {horizontal_alignment::flush, vertical_alignment::middle};
    }

    [[nodiscard]] static constexpr alignment middle_left() noexcept
    {
        return {horizontal_alignment::left, vertical_alignment::middle};
    }

    [[nodiscard]] static constexpr alignment middle_center() noexcept
    {
        return {horizontal_alignment::center, vertical_alignment::middle};
    }

    [[nodiscard]] static constexpr alignment middle_justified() noexcept
    {
        return {horizontal_alignment::justified, vertical_alignment::middle};
    }

    [[nodiscard]] static constexpr alignment middle_right() noexcept
    {
        return {horizontal_alignment::right, vertical_alignment::middle};
    }

    [[nodiscard]] static constexpr alignment bottom_left() noexcept
    {
        return {horizontal_alignment::left, vertical_alignment::bottom};
    }

    [[nodiscard]] static constexpr alignment bottom_center() noexcept
    {
        return {horizontal_alignment::center, vertical_alignment::bottom};
    }

    [[nodiscard]] static constexpr alignment bottom_right() noexcept
    {
        return {horizontal_alignment::right, vertical_alignment::bottom};
    }

    [[nodiscard]] constexpr horizontal_alignment text() const noexcept
    {
        return static_cast<horizontal_alignment>(_value & 0xf);
    }

    [[nodiscard]] constexpr vertical_alignment vertical() const noexcept
    {
        return static_cast<vertical_alignment>(_value >> 4);
    }

    [[nodiscard]] constexpr friend bool operator==(alignment const &lhs, alignment const &rhs) noexcept = default;

    [[nodiscard]] constexpr friend bool operator==(alignment const &lhs, horizontal_alignment const &rhs) noexcept
    {
        return lhs.text() == rhs;
    }

    [[nodiscard]] constexpr friend bool operator==(horizontal_alignment const &lhs, alignment const &rhs) noexcept
    {
        return lhs == rhs.text();
    }

    [[nodiscard]] constexpr friend bool operator==(alignment const &lhs, vertical_alignment const &rhs) noexcept
    {
        return lhs.vertical() == rhs;
    }

    [[nodiscard]] constexpr friend bool operator==(vertical_alignment const &lhs, alignment const &rhs) noexcept
    {
        return lhs == rhs.vertical();
    }

private:
    /** The combined vertical- and text-alignment
     *
     * [7:5] vertical_alignment
     * [4:0] horizontal_alignment
     */
    uint8_t _value;
};

/** Combine vertical and horizontal alignment.
 *
 * @param lhs A text alignment.
 * @param rhs A vertical alignment.
 * @return A combined vertical and horizontal alignment.
 */
constexpr alignment operator|(horizontal_alignment lhs, vertical_alignment rhs) noexcept
{
    return alignment{lhs, rhs};
}

/** Combine vertical and horizontal alignment.
 *
 * @param lhs A text alignment.
 * @param rhs A vertical alignment.
 * @return A combined vertical and horizontal alignment.
 */
constexpr alignment operator|(vertical_alignment lhs, horizontal_alignment rhs) noexcept
{
    return alignment{lhs, rhs};
}

} // namespace tt::inline v1
