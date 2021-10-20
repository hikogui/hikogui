// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../geometry/matrix.hpp"
#include "../geometry/axis_aligned_rectangle.hpp"
#include "../geometry/transform.hpp"
#include "../geometry/translate.hpp"

namespace tt {

class layout_context {
public:
    /** This matrix transforms local coordinates to the coordinates of the parent widget.
     */
    matrix3 to_parent;

    /** This matrix transforms parent widget's coordinates to local coordinates.
     */
    matrix3 from_parent;

    /** This matrix transforms local coordinates to window coordinates.
     */
    matrix3 to_window;

    /** This matrix transforms window coordinates to local coordinates.
     */
    matrix3 from_window;

    /** The rectangle of the widget.
     *
     * The left-bottom corner of the rectangle is at (0,0)
     *
     * @note widget's coordinate system.
     */
    aarectangle rectangle;

    /** The clipping rectangle.
     *
     * This is the rectangle that all drawing must be clipped to.
     * This rectangle often includes the margin, as widget may draw in their own margin.
     *
     * The clipping rectangle may be smaller than the widget, or even empty when the widget is
     * scrolled outside of the aperture of a scroll widget.
     *
     * @note widget's coordinate system.
     */
    aarectangle clipping_rectangle;

    /** The hit rectangle
     *
     * This rectangle is used to check if the hitbox test falls within the visual-area of
     * the widget. This rectangle was intersected by the clipping-rectangle.
     *
     * @note widget's coordinate system.
     */
    aarectangle hit_rectangle;

    /** The rectangle to use to request a redraw for the widget.
     *
     * @note window's coordinate system.
     */
    aarectangle redraw_rectangle;

    /** The layout created for displaying at this time point.
     */
    utc_nanoseconds display_time_point;

    constexpr layout_context(layout_context const &) noexcept = default;
    constexpr layout_context(layout_context &&) noexcept = default;
    constexpr layout_context &operator=(layout_context const &) noexcept = default;
    constexpr layout_context &operator=(layout_context &&) noexcept = default;

    [[nodiscard]] constexpr layout_context() noexcept :
        to_parent(),
        from_parent(),
        to_window(),
        from_window(),
        rectangle(),
        clipping_rectangle(),
        hit_rectangle(),
        redraw_rectangle(),
        display_time_point()
    {
    }

    /** Construct a layout_context from inside the window.
     */
    constexpr layout_context(extent2 window_size, utc_nanoseconds display_time_point) noexcept :
        to_parent(),
        from_parent(),
        to_window(),
        from_window(),
        rectangle(window_size),
        clipping_rectangle(window_size),
        hit_rectangle(window_size),
        redraw_rectangle(window_size),
        display_time_point(display_time_point)
    {
    }

    /** Create a new layout_context for the child widget.
     *
     * @param child_rectangle The location and size of the child widget, relative to the current widget.
     * @param elevation The relative elevation of the child widget compared to the current widget.
     * @return A new layout_context for use by the child widget.
     */
    [[nodiscard]] constexpr layout_context transform(aarectangle const &child_rectangle, float elevation = 1.0f) const noexcept
    {
        auto from_parent2 = ~translate2{child_rectangle};
        auto to_parent3 = translate3{child_rectangle, elevation};
        auto from_parent3 = ~to_parent3;

        layout_context r;
        r.to_parent = to_parent3;
        r.from_parent = from_parent3;
        r.to_window = to_parent3 * this->to_window;
        r.from_window = from_parent3 * this->from_window;
        r.rectangle = aarectangle{child_rectangle.size()};
        r.clipping_rectangle = from_parent2 * this->clipping_rectangle;
        r.hit_rectangle = intersect(r.rectangle, r.clipping_rectangle);
        r.redraw_rectangle = bounding_rectangle(r.to_window * (r.rectangle + 10.0f));
        r.display_time_point = this->display_time_point;
        return r;
    }

    /** Clip the context with the new clipping rectangle.
     *
     * The context's clipping rectangle is intersected with the new clipping rectangle.
     *
     * @param new_clipping_rectangle The new clipping rectangle.
     * @return A new context that is clipped..
     */
    [[nodiscard]] constexpr layout_context clip(aarectangle new_clipping_rectangle) const noexcept
    {
        auto r = *this;
        r.clipping_rectangle = intersect(r.clipping_rectangle, new_clipping_rectangle);
        r.hit_rectangle = intersect(r.hit_rectangle, new_clipping_rectangle);
        return r;
    }

    /** Override e context with the new clipping rectangle.
     *
     * @param new_clipping_rectangle The new clipping rectangle.
     * @return A new context that is clipped..
     */
    [[nodiscard]] constexpr layout_context override_clip(aarectangle new_clipping_rectangle) const noexcept
    {
        auto r = *this;
        r.clipping_rectangle = new_clipping_rectangle;
        r.hit_rectangle = new_clipping_rectangle;
        return r;
    }

    /** Compare if layouts are the same.
     *
     * It should not check the display_time_point because this value does not change
     * the layout.
     */
    [[nodiscard]] friend constexpr bool operator==(layout_context const &lhs, layout_context const &rhs) noexcept
    {
        // clang-format off
        return
            lhs.rectangle == rhs.rectangle and
            lhs.to_parent == rhs.to_parent and
            lhs.from_parent == rhs.from_parent and
            lhs.to_window == rhs.to_window and
            lhs.from_window == rhs.from_window and
            lhs.clipping_rectangle == rhs.clipping_rectangle and
            lhs.hit_rectangle == rhs.hit_rectangle and
            lhs.redraw_rectangle == rhs.redraw_rectangle;

        // clang-format on
    }

    [[nodiscard]] friend constexpr layout_context operator*(aarectangle const &lhs, layout_context const &rhs) noexcept
    {
        return rhs.transform(lhs);
    }
};

} // namespace tt
