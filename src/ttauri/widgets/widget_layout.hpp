// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../geometry/matrix.hpp"
#include "../geometry/axis_aligned_rectangle.hpp"
#include "../geometry/transform.hpp"
#include "../geometry/translate.hpp"
#include "../chrono.hpp"

namespace tt::inline v1 {

/** Result of widget_layout::store()
 */
enum class layout_update {
    /** The layout was unmodified.
     */
    none,

    /** One or more matrices, clipping, hit and redraw rectangle was modified.
     */
    transform,

    /** The size of the widget was modified.
     *
     * This state also implies `layout_update::transform`.
     */
    size
};

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

    constexpr widget_layout(widget_layout const &) noexcept = default;
    constexpr widget_layout(widget_layout &&) noexcept = default;
    constexpr widget_layout &operator=(widget_layout const &) noexcept = default;
    constexpr widget_layout &operator=(widget_layout &&) noexcept = default;
    constexpr widget_layout() noexcept = default;

    constexpr layout_update compare(widget_layout const &other) const noexcept
    {
        tt_axiom((to_parent == other.to_parent) == (from_parent == other.from_parent));
        tt_axiom((to_window == other.to_window) == (from_window == other.from_window));

        // clang-format off
        if (size != other.size) {
            return layout_update::size;

        } else if (
            to_parent != other.to_parent or
            to_window != other.to_window or
            clipping_rectangle != other.clipping_rectangle or
            hit_rectangle != other.hit_rectangle or
            redraw_rectangle != other.redraw_rectangle) {
            return layout_update::transform;

        } else {
            return layout_update::none;
        }
        // clang-format on
    }

    [[nodiscard]] constexpr aarectangle rectangle() const noexcept
    {
        return aarectangle{size};
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

    constexpr layout_update store(widget_layout const &other) noexcept
    {
        ttlet r = compare(other);
        if (r != layout_update::none) {
            *this = other;
        }
        return r;
    }

    /** Construct a widget_layout from inside the window.
     */
    constexpr widget_layout(extent2 window_size, utc_nanoseconds display_time_point) noexcept :
        to_parent(),
        from_parent(),
        to_window(),
        from_window(),
        size(window_size),
        clipping_rectangle(window_size),
        hit_rectangle(window_size),
        redraw_rectangle(window_size),
        display_time_point(display_time_point)
    {
    }

    /** Create a new widget_layout for the child widget.
     *
     * @param child_rectangle The location and size of the child widget, relative to the current widget.
     * @param elevation The relative elevation of the child widget compared to the current widget.
     * @return A new widget_layout for use by the child widget.
     */
    [[nodiscard]] constexpr widget_layout transform(aarectangle const &child_rectangle, float elevation = 1.0f) const noexcept
    {
        auto from_parent2 = ~translate2{child_rectangle};
        auto to_parent3 = translate3{child_rectangle, elevation};
        auto from_parent3 = ~to_parent3;

        widget_layout r;
        r.to_parent = to_parent3;
        r.from_parent = from_parent3;
        r.to_window = to_parent3 * this->to_window;
        r.from_window = from_parent3 * this->from_window;
        r.size = child_rectangle.size();
        r.clipping_rectangle = from_parent2 * this->clipping_rectangle;
        r.hit_rectangle = intersect(aarectangle{r.size}, r.clipping_rectangle);
        r.redraw_rectangle = bounding_rectangle(r.to_window * (aarectangle{r.size} + redraw_overhang));
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
    [[nodiscard]] constexpr widget_layout clip(aarectangle new_clipping_rectangle) const noexcept
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
    [[nodiscard]] constexpr widget_layout override_clip(aarectangle new_clipping_rectangle) const noexcept
    {
        auto r = *this;
        r.clipping_rectangle = new_clipping_rectangle;
        r.hit_rectangle = new_clipping_rectangle;
        return r;
    }

    [[nodiscard]] friend constexpr widget_layout operator*(aarectangle const &lhs, widget_layout const &rhs) noexcept
    {
        return rhs.transform(lhs);
    }
};

} // namespace tt::inline v1
