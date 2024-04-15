// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../macros.hpp"
#include "../unit/unit.hpp"
#include <algorithm>
#include <cmath>

hi_export_module(hikogui.font.font_metrics);

hi_export namespace hi::inline v1 {

/** The metrics of a font.
 *
 * These are the metrics that are used for the font as a whole.
 * Inside the font these are in 'em' units, outside the font they
 * may have been scaled to 'points' or 'dp'.
 */
hi_export struct font_metrics {
    /** Distance from baseline of highest ascender.
     */
    float ascender = 0.0f;

    /** Distance from baseline of lowest descender.
     *
     * @note positive downward.
     */
    float descender = 0.0f;

    /** Distance between lines.
     *
     * The distance between the descender of a line and the ascender of the next line.
     */
    float line_gap = 0.0f;

    /** Height of capital letter, or height of the letter 'H'.
     */
    float cap_height = 0.0f;

    /** Height of lower case characters without ascenders or descenders, or the small letter 'x'.
     */
    float x_height = 0.0f;

    /** The advance for digits, specifically the digit '8'.
     *
     * @note: All digits in a font should have the same advance.
     */
    float digit_advance = 0.0f;

    /** The multiplier for the space between lines of the same paragraph.
     * @note This is not an actual font property, but comes from the text-style.
     */
    float line_spacing = 1.0f;

    /** The multiplier for the space between two paragraphs.
     * @note This is not an actual font property, but comes from the text-style.
     */
    float paragraph_spacing = 1.5f;

    ~font_metrics() = default;
    constexpr font_metrics() noexcept = default;
    constexpr font_metrics(font_metrics const&) noexcept = default;
    constexpr font_metrics(font_metrics&&) noexcept = default;
    constexpr font_metrics& operator=(font_metrics const&) noexcept = default;
    constexpr font_metrics& operator=(font_metrics&&) noexcept = default;
    [[nodiscard]] constexpr friend bool operator==(font_metrics const&, font_metrics const&) noexcept = default;

    /** Scale the metrics by a scalar value.
     */
    [[nodiscard]] constexpr friend font_metrics operator*(float const& lhs, font_metrics const& rhs) noexcept
    {
        font_metrics r;
        r.ascender = lhs * rhs.ascender;
        r.descender = lhs * rhs.descender;
        r.line_gap = lhs * rhs.line_gap;
        r.cap_height = lhs * rhs.cap_height;
        r.x_height = lhs * rhs.x_height;
        r.digit_advance = lhs * rhs.digit_advance;
        return r;
    }

    [[nodiscard]] constexpr friend font_metrics max(font_metrics const& a, font_metrics const& b) noexcept
    {
        font_metrics r;
        r.ascender = std::max(a.ascender, b.ascender);
        r.descender = std::max(a.descender, b.descender);
        r.line_gap = std::max(a.line_gap, b.line_gap);
        r.cap_height = std::max(a.cap_height, b.cap_height);
        r.x_height = std::max(a.x_height, b.x_height);
        r.digit_advance = std::max(a.digit_advance, b.digit_advance);
        return r;
    }

    /** Round to font size in pixels so that the scaled x-height is an integral.
     */
    [[nodiscard]] pixels_f round_size(pixels_f font_size) const noexcept
    {
        auto const x_height_in_pixel = round_as(pixels, em_squares(x_height) * font_size));
        auto const rounded_size = x_height_in_pixel / x_height;
        return rounded_size;
    }
};

} // namespace hi::inline v1
