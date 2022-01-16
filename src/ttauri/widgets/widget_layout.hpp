// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../geometry/matrix.hpp"
#include "../geometry/axis_aligned_rectangle.hpp"
#include "../geometry/transform.hpp"
#include "../geometry/translate.hpp"
#include "../text/unicode_bidi_class.hpp"
#include "../GFX/sub_pixel_orientation.hpp"
#include "../chrono.hpp"

namespace tt::inline v1 {

class widget_layout {
public:
    /** The amount of pixels that the redraw request will overhang the widget.
     *
     * Widgets are allowed to draw inside their margins, in most cases this will just be a border.
     */
    static constexpr float redraw_overhang = 2.0f;

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

    /** Size of the widget.
     */
    extent2 size;

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

    /** The size of a sub-pixel.
     *
     * @note the sub-pixel-size is represented in the widget's coordinate system.
     */
    extent2 sub_pixel_size;

    /** The default writing direction.
     *
     * @note Must be either `L` or `R`.
     */
    unicode_bidi_class writing_direction;

    /** The layout created for displaying at this time point.
     */
    utc_nanoseconds display_time_point;

    constexpr widget_layout(widget_layout const &) noexcept = default;
    constexpr widget_layout(widget_layout &&) noexcept = default;
    constexpr widget_layout &operator=(widget_layout const &) noexcept = default;
    constexpr widget_layout &operator=(widget_layout &&) noexcept = default;
    constexpr widget_layout() noexcept = default;

    [[nodiscard]] constexpr friend bool operator==(widget_layout const &lhs, widget_layout const &rhs) noexcept
    {
        tt_axiom((lhs.to_parent == rhs.to_parent) == (lhs.from_parent == rhs.from_parent));
        tt_axiom((lhs.to_window == rhs.to_window) == (lhs.from_window == rhs.from_window));

        // clang-format on
        return
            lhs.size == rhs.size and
            lhs.to_parent == rhs.to_parent and
            lhs.to_window == rhs.to_window and
            lhs.clipping_rectangle == rhs.clipping_rectangle and
            lhs.sub_pixel_size == rhs.sub_pixel_size and
            lhs.writing_direction == rhs.writing_direction;
        // clang-format off
    }

    /** Check if the mouse position is inside the widget.
     *
     * @param mouse_position The mouse position in local coordinates.
     * @return True if the mouse position is on the widget and is not clipped.
     */
    [[nodiscard]] constexpr bool contains(point3 mouse_position) const noexcept
    {
        return rectangle().contains(mouse_position) and clipping_rectangle.contains(mouse_position);
    }

    [[nodiscard]] constexpr aarectangle rectangle() const noexcept
    {
        return aarectangle{size};
    }

    /** Get the clipping rectangle in window coordinate system.
     */
    [[nodiscard]] constexpr aarectangle window_clipping_rectangle() const noexcept
    {
        return bounding_rectangle(to_window * clipping_rectangle);
    }

    /** Get the clipping rectangle in window coordinate system.
     *
     * @param narrow_clipping_rectangle A clipping rectangle in local coordinate
     *        system that will be intersected with the layout's clipping rectangle.
     */
    [[nodiscard]] constexpr aarectangle window_clipping_rectangle(aarectangle narrow_clipping_rectangle) const noexcept
    {
        return bounding_rectangle(to_window * intersect(clipping_rectangle, narrow_clipping_rectangle));
    }

    [[nodiscard]] constexpr float width() const noexcept
    {
        return size.width();
    }

    [[nodiscard]] constexpr float height() const noexcept
    {
        return size.height();
    }

    [[nodiscard]] constexpr float base_line() const noexcept
    {
        return size.height() * 0.5f;
    }

    /** Construct a widget_layout from inside the window.
     */
    constexpr widget_layout(
        extent2 window_size,
        tt::sub_pixel_orientation sub_pixel_orientation,
        unicode_bidi_class writing_direction,
        utc_nanoseconds display_time_point) noexcept :
        to_parent(),
        from_parent(),
        to_window(),
        from_window(),
        size(window_size),
        clipping_rectangle(window_size),
        sub_pixel_size(tt::sub_pixel_size(sub_pixel_orientation)),
        writing_direction(writing_direction),
        display_time_point(display_time_point)
    {
    }

    /** Create a new widget_layout for the child widget.
     *
     * @param child_rectangle The location and size of the child widget, relative to the current widget.
     * @param elevation The elevation of the child widget, relative to the current widget.
     * @param new_clipping_rectangle The new clipping rectangle of the child widget, relative to the current widget.
     * @return A new widget_layout for use by the child widget.
     */
    [[nodiscard]] constexpr widget_layout
    transform(aarectangle const &child_rectangle, float elevation, aarectangle new_clipping_rectangle) const noexcept
    {
        auto to_parent3 = translate3{child_rectangle, elevation};
        auto from_parent3 = ~to_parent3;

        widget_layout r;
        r.to_parent = to_parent3;
        r.from_parent = from_parent3;
        r.to_window = to_parent3 * this->to_window;
        r.from_window = from_parent3 * this->from_window;
        r.size = child_rectangle.size();
        r.clipping_rectangle = bounding_rectangle(from_parent3 * intersect(this->clipping_rectangle, new_clipping_rectangle));
        r.sub_pixel_size = this->sub_pixel_size;
        r.writing_direction = this->writing_direction;
        r.display_time_point = this->display_time_point;
        return r;
    }

    /** Create a new widget_layout for the child widget.
     *
     * @param child_rectangle The location and size of the child widget, relative to the current widget.
     * @param elevation The relative elevation of the child widget compared to the current widget.
     * @return A new widget_layout for use by the child widget.
     */
    [[nodiscard]] constexpr widget_layout transform(aarectangle const &child_rectangle, float elevation = 1.0f) const noexcept
    {
        return transform(child_rectangle, elevation, child_rectangle + redraw_overhang);
    }

    /** Override e context with the new clipping rectangle.
     *
     * @param new_clipping_rectangle The new clipping rectangle.
     * @return A new context that is clipped..
     */
    [[nodiscard]] constexpr widget_layout override_clip(aarectangle new_clipping_rectangle) const noexcept
    {
        auto r = *this;
        r.clipping_rectangle = new_clipping_rectangle;
        return r;
    }
};

} // namespace tt::inline v1
