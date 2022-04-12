// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

namespace hi::inline v1 {

/** The metrics of a font.
 *
 * These are the metrics that are used for the font as a whole.
 * Inside the font these are in 'em' units, outside the font they
 * may have been scaled to 'points' or 'dp'.
 */
struct font_metrics {
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

    /** Scale the metrics by a scalar value.
     */
    [[nodiscard]] constexpr friend font_metrics operator*(float const &lhs, font_metrics const &rhs) noexcept
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

    [[nodiscard]] constexpr friend font_metrics max(font_metrics const &a, font_metrics const &b) noexcept
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

    /** Round a scale so that the scaled x-height is an integral.
     */
    [[nodiscard]] float round_scale(float size) const noexcept
    {
        return std::round(x_height * size) / x_height;
    }
};

} // namespace hi::inline v1