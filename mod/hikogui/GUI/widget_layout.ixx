// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/widget_layout.hpp Defines widget_layout.
 * @ingroup widget_utilities
 */

module;
#include "../macros.hpp"


export module hikogui_GUI : widget_layout;
import : gui_window_size;
import hikogui_geometry;
import hikogui_layout;
import hikogui_settings;
import hikogui_time;
import hikogui_utility;

export namespace hi { inline namespace v1 {

enum class transform_command {
    /** The child widget stays at the same elevation and layer.
     */
    level,

    /** The child widget increments to the next elevation and layer.
     */
    increment,

    /** The child widget increments to the next elevation but layer stays the same.
     */
    menu_item,

    /** The child widget increases the elevation by 20 and resets the layer.
     */
    overlay
};

/** The layout of a widget.
 *
 * This object is created by a container to position a child-widget
 * within it.
 *
 * The layout includes:
 *  - the size of the widget.
 *  - translation matrices between the parent and child widget.
 *  - translation matrices between the child widget and the window.
 *  - the clipping rectangle when the parent only wants to display a part the child.
 *  - if the widget should display itself in left-to-right or right-to-left language mode.
 *  - the baseline where text should be drawn.
 *
 * @ingroup widget_utilities
 */
class widget_layout {
public:
    /** The amount of pixels that the redraw request will overhang the widget.
     *
     * Widgets are allowed to draw inside their margins, in most cases this will just be a border.
     */
    constexpr static int redraw_overhang = 2;

    /** Shape of the widget.
     * Since a widget_layout is always in local coordinates, the `left` and `bottom` values are zero.
     */
    box_shape shape;

    /** This matrix transforms local coordinates to the coordinates of the parent widget.
     */
    translate2 to_parent = {};

    /** This matrix transforms parent widget's coordinates to local coordinates.
     */
    translate2 from_parent = {};

    /** This matrix transforms local coordinates to window coordinates.
     */
    translate2 to_window = {};

    /** This matrix transforms window coordinates to local coordinates.
     */
    translate2 from_window = {};

    /** Size of the window.
     */
    extent2 window_size = {};

    /** The size state of the window.
     */
    gui_window_size window_size_state = gui_window_size::normal;

    /** The elevation of the widget above the window.
     */
    float elevation = 0.0f;

    /** The number of visible layers above the window.
     *
     * The layer value is used to determine what colors are used
     * for drawing the widget, in a nice step-pattern.
     *
     * Layer is set as followed:
     * - Widgets that draw anything increment the layer by 1.
     * - Many container widgets do not increment the layer.
     * - Overlays will reset the layer to 0.
     */
    int layer = 0;

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
    aarectangle clipping_rectangle = {};

    /** The size of a sub-pixel.
     *
     * @note the sub-pixel-size is represented in the widget's coordinate system.
     */
    extent2 sub_pixel_size = {1.0f, 1.0f};

    /** The layout created for displaying at this time point.
     */
    utc_nanoseconds display_time_point = {};

    constexpr widget_layout(widget_layout const&) noexcept = default;
    constexpr widget_layout(widget_layout&&) noexcept = default;
    constexpr widget_layout& operator=(widget_layout const&) noexcept = default;
    constexpr widget_layout& operator=(widget_layout&&) noexcept = default;
    constexpr widget_layout() noexcept = default;
    [[nodiscard]] constexpr friend bool operator==(widget_layout const&, widget_layout const&) noexcept = default;

    /** Construct a widget_layout from inside the window.
     */
    constexpr widget_layout(
        extent2 window_size,
        gui_window_size window_size_state,
        hi::subpixel_orientation subpixel_orientation,
        utc_nanoseconds display_time_point) noexcept :
        to_parent(),
        from_parent(),
        to_window(),
        from_window(),
        shape(window_size),
        window_size(window_size),
        window_size_state(window_size_state),
        clipping_rectangle(window_size),
        sub_pixel_size(hi::sub_pixel_size(subpixel_orientation)),
        display_time_point(display_time_point)
    {
    }

    [[nodiscard]] constexpr bool empty() const noexcept
    {
        // Theme must always be set if layout is valid.
        return display_time_point == utc_nanoseconds{};
    }

    [[nodiscard]] constexpr explicit operator bool() const noexcept
    {
        return not empty();
    }

    [[nodiscard]] constexpr translate3 to_window3() const noexcept
    {
        return translate3{to_window, elevation};
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
        return shape.rectangle;
    }

    /** Get the rectangle in window coordinate system.
     */
    [[nodiscard]] constexpr aarectangle rectangle_on_window() const noexcept
    {
        return to_window * rectangle();
    }

    /** Get the clipping rectangle in window coordinate system.
     */
    [[nodiscard]] constexpr aarectangle clipping_rectangle_on_window() const noexcept
    {
        return to_window * clipping_rectangle;
    }

    /** Get the clipping rectangle in window coordinate system.
     *
     * @param narrow_clipping_rectangle A clipping rectangle in local coordinate
     *        system that will be intersected with the layout's clipping rectangle.
     */
    [[nodiscard]] constexpr aarectangle clipping_rectangle_on_window(aarectangle narrow_clipping_rectangle) const noexcept
    {
        return to_window * intersect(clipping_rectangle, narrow_clipping_rectangle);
    }

    [[nodiscard]] constexpr float width() const noexcept
    {
        return shape.width();
    }

    [[nodiscard]] constexpr float height() const noexcept
    {
        return shape.height();
    }

    [[nodiscard]] constexpr extent2 size() const noexcept
    {
        return shape.size();
    }

    /** Create a new widget_layout for the child widget.
     *
     * @param child_shape The location and size of the child widget, relative to the current widget.
     * @param command Command to how the elevation and layer are transformed.
     * @param new_clipping_rectangle The new clipping rectangle of the child widget, relative to the current widget.
     * @return A new widget_layout for use by the child widget.
     */
    [[nodiscard]] constexpr widget_layout
    transform(box_shape const& child_shape, transform_command command, aarectangle new_clipping_rectangle) const noexcept
    {
        widget_layout r = *this;
        r.shape.rectangle = aarectangle{child_shape.size()};

        if (child_shape.baseline) {
            r.shape.baseline = *child_shape.baseline - child_shape.y();

        } else if (r.shape.baseline) {
            // Use the baseline of the current layout and translate it.
            *r.shape.baseline -= child_shape.y();
        }

        if (child_shape.centerline) {
            r.shape.centerline = *child_shape.centerline - child_shape.x();

        } else if (r.shape.centerline) {
            // Use the baseline of the current layout and translate it.
            *r.shape.centerline -= child_shape.x();
        }

        r.to_parent = translate2{child_shape.x(), child_shape.y()};
        r.from_parent = ~r.to_parent;
        r.to_window = r.to_parent * this->to_window;
        r.from_window = r.from_parent * this->from_window;
        r.clipping_rectangle = r.from_parent * intersect(this->clipping_rectangle, new_clipping_rectangle);

        switch (command) {
        case transform_command::level:
            r.elevation += 0.0f;
            r.layer += 0;
            break;
        case transform_command::increment:
            r.elevation += 1.0f;
            r.layer += 1;
            break;
        case transform_command::menu_item:
            r.elevation += 1.0f;
            r.layer += 0;
            break;
        case transform_command::overlay:
            r.elevation += 20.0f;
            r.layer = 0;
            break;
        default:
            hi_no_default();
        }
        return r;
    }

    /** Create a new widget_layout for the child widget.
     *
     * @param child_shape The location and size of the child widget, relative to the current widget.
     * @param command Command to how the elevation and layer are transformed.
     * @return A new widget_layout for use by the child widget.
     */
    [[nodiscard]] constexpr widget_layout
    transform(box_shape const& child_shape, transform_command command = transform_command::increment) const noexcept
    {
        return transform(child_shape, command, child_shape.rectangle + redraw_overhang);
    }

    /** Create a new widget_layout for the child widget.
     *
     * @param child_shape The location and size of the child widget, relative to the current widget.
     * @param new_clipping_rectangle The new clipping rectangle of the child widget, relative to the current widget.
     * @return A new widget_layout for use by the child widget.
     */
    [[nodiscard]] constexpr widget_layout
    transform(box_shape const& child_shape, aarectangle new_clipping_rectangle) const noexcept
    {
        return transform(child_shape, transform_command::increment, new_clipping_rectangle);
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

}} // namespace hi::v1
