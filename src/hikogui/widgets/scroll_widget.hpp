// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/scroll_widget.hpp Defines scroll_widget.
 * @ingroup widgets
 */

#pragma once

#include "widget.hpp"
#include "scroll_bar_widget.hpp"
#include "scroll_aperture_widget.hpp"
#include "../GUI/gui_window.hpp"
#include "../geometry/axis.hpp"

namespace hi { inline namespace v1 {

/** The scroll widget allows a content widget to be shown in less space than is
 * required.
 *
 *  The user can then show the part of the content widget by using the
 * scroll-bar widgets which are provided by the scroll widget.
 *
 * The size of the scroll widget is based on the `widget::minimum_size()`,
 * `widget::preferred_size()` and `widget::maximum_size()` of the content widget
 * together with the space needed for the scrollbars.
 *
 * In the directions that are allowed for scrolling the minimum size of the
 * scroll widget is adjusted to be much smaller, up to the smallest size of the
 * scroll-bar widget in that direction.
 *
 * Scroll-bars are automatically added when the actual size of the scroll widget
 * is smaller than the content, this will happen even if the template parameters
 * given did not allow scrolling in that direction. This is useful behavior when
 * the scroll widget is part of an overlay widget which was unable to size to
 * the minimum size requested.
 *
 * @image html scroll_widget.png
 *
 * @ingroup widgets
 * @tparam Axis the axis that the content may be scrolled. Allowed values are
 *              `axis::horizontal`, `axis::vertical` or `axis::both`.
 * @tparam ControlsWindow If set to true, when the content changes size the
 *                        window gets a signal to resize to its preferred size.
 */
template<axis Axis = axis::both, bool ControlsWindow = false>
class scroll_widget final : public widget {
public:
    using super = widget;

    static constexpr hi::axis axis = Axis;
    static constexpr bool controls_window = ControlsWindow;

    ~scroll_widget() {}

    /** Constructs an empty scroll widget.
     *
     * @param window The window.
     * @param parent The parent widget.
     */
    scroll_widget(gui_window& window, widget *parent) noexcept : super(window, parent)
    {
        hi_axiom(is_gui_thread());
        hi_assert_not_null(parent);

        // The scroll-widget will not draw itself, only its selected content.
        semantic_layer = parent->semantic_layer;

        _aperture = std::make_unique<scroll_aperture_widget>(window, this);
        _horizontal_scroll_bar = std::make_unique<horizontal_scroll_bar_widget>(
            window, this, _aperture->content_width, _aperture->aperture_width, _aperture->offset_x);
        _vertical_scroll_bar = std::make_unique<vertical_scroll_bar_widget>(
            window, this, _aperture->content_height, _aperture->aperture_height, _aperture->offset_y);
    }

    /** Add a content widget directly to this scroll widget.
     *
     * This widget is added as the content widget.
     *
     * @pre No content widgets have been added before.
     * @tparam Widget The type of the widget to be constructed.
     * @param args The arguments passed to the constructor of the widget.
     * @return A reference to the widget that was created.
     */
    template<typename Widget, typename... Args>
    Widget& make_widget(Args&&...args) noexcept
    {
        return _aperture->make_widget<Widget>(std::forward<Args>(args)...);
    }

    /// @privatesection
    [[nodiscard]] generator<widget *> children() const noexcept override
    {
        co_yield _aperture.get();
        co_yield _vertical_scroll_bar.get();
        co_yield _horizontal_scroll_bar.get();
    }

    widget_constraints const& set_constraints() noexcept override
    {
        _layout = {};
        hilet aperture_constraints = _aperture->set_constraints();
        hilet horizontal_constraints = _horizontal_scroll_bar->set_constraints();
        hilet vertical_constraints = _vertical_scroll_bar->set_constraints();

        _constraints = aperture_constraints;

        // When there are scrollbars the widget minimum size becomes basically zero.
        // However we should at least have enough room to fit in the scroll-bars length-wise.
        if constexpr (any(axis & axis::horizontal)) {
            _constraints.minimum.width() = horizontal_constraints.minimum.width();
            inplace_max(_constraints.preferred.width(), horizontal_constraints.minimum.width());
            inplace_max(_constraints.maximum.width(), horizontal_constraints.minimum.width());
        }
        if constexpr (any(axis & axis::vertical)) {
            _constraints.minimum.height() = vertical_constraints.minimum.height();
            inplace_max(_constraints.preferred.height(), vertical_constraints.minimum.height());
            inplace_max(_constraints.maximum.height(), vertical_constraints.minimum.height());
        }

        // Make room for the thickness of the scroll-bars.
        if constexpr (any(axis & axis::horizontal)) {
            _constraints.minimum.height() += horizontal_constraints.preferred.height();
            _constraints.preferred.height() += horizontal_constraints.preferred.height();
            _constraints.maximum.height() += horizontal_constraints.preferred.height();
        }
        if constexpr (any(axis & axis::vertical)) {
            _constraints.minimum.width() += vertical_constraints.preferred.width();
            _constraints.preferred.width() += vertical_constraints.preferred.width();
            _constraints.maximum.width() += vertical_constraints.preferred.width();
        }
        return _constraints;
    }

    void set_layout(widget_layout const& layout) noexcept override
    {
        if (compare_store(_layout, layout)) {
            hilet horizontal_visible = _aperture->x_axis_scrolls() and any(axis & axis::horizontal);
            hilet vertical_visible = _aperture->y_axis_scrolls() and any(axis & axis::vertical);
            hilet both_visible = horizontal_visible and vertical_visible;

            _horizontal_scroll_bar->mode = horizontal_visible ? widget_mode::enabled : widget_mode::invisible;
            _vertical_scroll_bar->mode = vertical_visible ? widget_mode::enabled : widget_mode::invisible;

            hilet vertical_scroll_bar_width = _vertical_scroll_bar->constraints().preferred.width();
            hilet horizontal_scroll_bar_height = _horizontal_scroll_bar->constraints().preferred.height();

            // The aperture size grows to fill the size of the layout.
            hilet aperture_size = extent2{
                vertical_visible ? layout.width() - vertical_scroll_bar_width : layout.width(),
                horizontal_visible ? layout.height() - horizontal_scroll_bar_height : layout.height()};

            hilet aperture_x = layout.left_to_right() ? 0.0f : vertical_visible ? vertical_scroll_bar_width : 0.0f;
            hilet aperture_y = horizontal_visible ? horizontal_scroll_bar_height : 0.0f;

            hilet aperture_offset = point2{aperture_x, aperture_y};
            _aperture_rectangle = aarectangle{aperture_offset, aperture_size};

            // The length of the scroll-bar is the full length of the widget, or just the length of the aperture depending
            // if the counter-part scroll-bar is visible.
            hilet horizontal_scroll_bar_size =
                extent2{both_visible ? aperture_size.width() : layout.width(), horizontal_scroll_bar_height};
            hilet vertical_scroll_bar_size =
                extent2{vertical_scroll_bar_width, both_visible ? aperture_size.height() : layout.height()};

            hilet vertical_scroll_bar_x = layout.left_to_right() ? layout.width() - vertical_scroll_bar_size.width() : 0.0f;
            hilet vertical_scroll_bar_y = layout.height() - vertical_scroll_bar_size.height();
            _vertical_scroll_bar_rectangle =
                aarectangle{point2{vertical_scroll_bar_x, vertical_scroll_bar_y}, vertical_scroll_bar_size};

            hilet horizontal_scroll_bar_x = layout.left_to_right() ? 0.0f : vertical_visible ? vertical_scroll_bar_width : 0.0f;
            hilet horizontal_scroll_bar_y = 0.0f;
            _horizontal_scroll_bar_rectangle =
                aarectangle{point2{horizontal_scroll_bar_x, horizontal_scroll_bar_y}, horizontal_scroll_bar_size};

            if constexpr (controls_window) {
                if (layout.left_to_right()) {
                    window.set_resize_border_priority(true, not vertical_visible, not horizontal_visible, true);
                } else {
                    window.set_resize_border_priority(not vertical_visible, true, not horizontal_visible, true);
                }
            }
        }

        _aperture->set_layout(layout.transform(_aperture_rectangle));
        if (*_vertical_scroll_bar->mode > widget_mode::invisible) {
            _vertical_scroll_bar->set_layout(layout.transform(_vertical_scroll_bar_rectangle));
        }
        if (*_horizontal_scroll_bar->mode > widget_mode::invisible) {
            _horizontal_scroll_bar->set_layout(layout.transform(_horizontal_scroll_bar_rectangle));
        }
    }

    void draw(draw_context const& context) noexcept
    {
        if (*mode > widget_mode::invisible) {
            _vertical_scroll_bar->draw(context);
            _horizontal_scroll_bar->draw(context);
            _aperture->draw(context);
        }
    }

    [[nodiscard]] hitbox hitbox_test(point3 position) const noexcept override
    {
        hi_axiom(is_gui_thread());

        if (*mode >= widget_mode::partial) {
            auto r = _aperture->hitbox_test_from_parent(position);
            r = _horizontal_scroll_bar->hitbox_test_from_parent(position, r);
            r = _vertical_scroll_bar->hitbox_test_from_parent(position, r);

            if (layout().contains(position)) {
                r = std::max(r, hitbox{this, position});
            }
            return r;

        } else {
            return {};
        }
    }
    // @endprivatesection
private:
    aarectangle _aperture_rectangle;
    std::unique_ptr<scroll_aperture_widget> _aperture;

    aarectangle _horizontal_scroll_bar_rectangle;
    std::unique_ptr<horizontal_scroll_bar_widget> _horizontal_scroll_bar;

    aarectangle _vertical_scroll_bar_rectangle;
    std::unique_ptr<vertical_scroll_bar_widget> _vertical_scroll_bar;
};

/** Vertical scroll widget.
 * A scroll widget that only scrolls vertically.
 *
 * @ingroup widgets
 * @see scroll_widget
 * @tparam ControlsWindow If set to true, when the content changes size the
 * window gets a signal to resize to its preferred size.
 */
template<bool ControlsWindow = false>
using vertical_scroll_widget = scroll_widget<axis::vertical, ControlsWindow>;

/** Horizontal scroll widget.
 * A scroll widget that only scrolls horizontally.
 *
 * @ingroup widgets
 * @see scroll_widget
 * @tparam ControlsWindow If set to true, when the content changes size the
 * window gets a signal to resize to its preferred size.
 */
template<bool ControlsWindow = false>
using horizontal_scroll_widget = scroll_widget<axis::horizontal, ControlsWindow>;

}} // namespace hi::v1
