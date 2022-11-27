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
 */
template<axis Axis = axis::both>
class scroll_widget final : public widget {
public:
    using super = widget;

    static constexpr hi::axis axis = Axis;

    ~scroll_widget() {}

    /** Constructs an empty scroll widget.
     *
     * @param parent The parent widget.
     */
    scroll_widget(widget *parent) noexcept : super(parent)
    {
        hi_axiom(loop::main().on_thread());
        hi_assert_not_null(parent);

        // The scroll-widget will not draw itself, only its selected content.
        semantic_layer = parent->semantic_layer;

        _aperture = std::make_unique<scroll_aperture_widget>(this);
        _horizontal_scroll_bar = std::make_unique<horizontal_scroll_bar_widget>(
            this, _aperture->content_width, _aperture->aperture_width, _aperture->offset_x);
        _vertical_scroll_bar = std::make_unique<vertical_scroll_bar_widget>(
            this, _aperture->content_height, _aperture->aperture_height, _aperture->offset_y);
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

    box_constraints const& set_constraints(set_constraints_context const& context) noexcept override
    {
        _layout = {};
        _aperture_constraints = _aperture->set_constraints(context);
        _horizontal_scroll_bar_constraints = _horizontal_scroll_bar->set_constraints(context);
        _vertical_scroll_bar_constraints = _vertical_scroll_bar->set_constraints(context);

        _constraints = _aperture_constraints;

        // When there are scrollbars the widget minimum size becomes basically zero.
        // However we should at least have enough room to fit in the scroll-bars length-wise.
        if constexpr (to_bool(axis & axis::horizontal)) {
            _constraints.minimum_width = _horizontal_scroll_bar_constraints.minimum_width;
            inplace_max(_constraints.preferred_width, _horizontal_scroll_bar_constraints.minimum_width);
            inplace_max(_constraints.maximum_width, _horizontal_scroll_bar_constraints.minimum_width);
        }
        if constexpr (to_bool(axis & axis::vertical)) {
            _constraints.minimum_height = _vertical_scroll_bar_constraints.minimum_height;
            inplace_max(_constraints.preferred_height, _vertical_scroll_bar_constraints.minimum_height);
            inplace_max(_constraints.maximum_height, _vertical_scroll_bar_constraints.minimum_height);
        }

        // Make room for the thickness of the scroll-bars.
        if constexpr (to_bool(axis & axis::horizontal)) {
            _constraints.minimum_height += _horizontal_scroll_bar_constraints.preferred_height;
            _constraints.preferred_height += _horizontal_scroll_bar_constraints.preferred_height;
            _constraints.maximum_height += _horizontal_scroll_bar_constraints.preferred_height;
        }
        if constexpr (to_bool(axis & axis::vertical)) {
            _constraints.minimum_width += _vertical_scroll_bar_constraints.preferred_width;
            _constraints.preferred_width += _vertical_scroll_bar_constraints.preferred_width;
            _constraints.maximum_width += _vertical_scroll_bar_constraints.preferred_width;
        }
        return _constraints;
    }

    void set_layout(widget_layout const& context) noexcept override
    {
        if (compare_store(_layout, context)) {
            hilet horizontal_visible = _aperture->x_axis_scrolls() and to_bool(axis & axis::horizontal);
            hilet vertical_visible = _aperture->y_axis_scrolls() and to_bool(axis & axis::vertical);
            hilet both_visible = horizontal_visible and vertical_visible;

            _horizontal_scroll_bar->mode = horizontal_visible ? widget_mode::enabled : widget_mode::invisible;
            _vertical_scroll_bar->mode = vertical_visible ? widget_mode::enabled : widget_mode::invisible;

            hilet vertical_scroll_bar_width = narrow_cast<float>(_vertical_scroll_bar_constraints.preferred_width);
            hilet horizontal_scroll_bar_height = narrow_cast<float>(_horizontal_scroll_bar_constraints.preferred_height);

            // The aperture size grows to fill the size of the layout.
            hilet aperture_size = extent2{
                vertical_visible ? narrow_cast<float>(context.width()) - vertical_scroll_bar_width :
                                   narrow_cast<float>(context.width()),
                horizontal_visible ? narrow_cast<float>(context.height()) - horizontal_scroll_bar_height :
                                     narrow_cast<float>(context.height())};

            hilet aperture_x = context.left_to_right() ? 0.0f : vertical_visible ? vertical_scroll_bar_width : 0.0f;
            hilet aperture_y = horizontal_visible ? horizontal_scroll_bar_height : 0.0f;

            hilet aperture_offset = point2{aperture_x, aperture_y};
            hilet aperture_rectangle = aarectangle{aperture_offset, aperture_size};
            _aperture_shape = box_shape{_aperture_constraints, aperture_rectangle, context.theme->baseline_adjustment};

            // The length of the scroll-bar is the full length of the widget, or just the length of the aperture depending
            // if the counter-part scroll-bar is visible.
            hilet horizontal_scroll_bar_size =
                extent2{both_visible ? aperture_size.width() : narrow_cast<float>(context.width()), horizontal_scroll_bar_height};
            hilet vertical_scroll_bar_size =
                extent2{vertical_scroll_bar_width, both_visible ? aperture_size.height() : narrow_cast<float>(context.height())};

            hilet vertical_scroll_bar_x =
                context.left_to_right() ? narrow_cast<float>(context.width()) - vertical_scroll_bar_size.width() : 0.0f;
            hilet vertical_scroll_bar_y = narrow_cast<float>(context.height()) - vertical_scroll_bar_size.height();
            hilet vertical_scroll_bar_rectangle =
                aarectangle{point2{vertical_scroll_bar_x, vertical_scroll_bar_y}, vertical_scroll_bar_size};
            _vertical_scroll_bar_shape =
                box_shape{_vertical_scroll_bar_constraints, vertical_scroll_bar_rectangle, context.theme->baseline_adjustment};

            hilet horizontal_scroll_bar_x = context.left_to_right() ? 0.0f : vertical_visible ? vertical_scroll_bar_width : 0.0f;
            hilet horizontal_scroll_bar_y = 0.0f;
            hilet horizontal_scroll_bar_rectangle =
                aarectangle{point2{horizontal_scroll_bar_x, horizontal_scroll_bar_y}, horizontal_scroll_bar_size};
            _horizontal_scroll_bar_shape = box_shape{
                _horizontal_scroll_bar_constraints, horizontal_scroll_bar_rectangle, context.theme->baseline_adjustment};
        }

        _aperture->set_layout(context.transform(_aperture_shape));
        if (*_vertical_scroll_bar->mode > widget_mode::invisible) {
            _vertical_scroll_bar->set_layout(context.transform(_vertical_scroll_bar_shape));
        }
        if (*_horizontal_scroll_bar->mode > widget_mode::invisible) {
            _horizontal_scroll_bar->set_layout(context.transform(_horizontal_scroll_bar_shape));
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
        hi_axiom(loop::main().on_thread());

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
    std::unique_ptr<scroll_aperture_widget> _aperture;
    box_constraints _aperture_constraints;
    box_shape _aperture_shape;

    std::unique_ptr<horizontal_scroll_bar_widget> _horizontal_scroll_bar;
    box_constraints _horizontal_scroll_bar_constraints;
    box_shape _horizontal_scroll_bar_shape;

    std::unique_ptr<vertical_scroll_bar_widget> _vertical_scroll_bar;
    box_constraints _vertical_scroll_bar_constraints;
    box_shape _vertical_scroll_bar_shape;
};

/** Vertical scroll widget.
 * A scroll widget that only scrolls vertically.
 *
 * @ingroup widgets
 * @see scroll_widget
 * window gets a signal to resize to its preferred size.
 */
using vertical_scroll_widget = scroll_widget<axis::vertical>;

/** Horizontal scroll widget.
 * A scroll widget that only scrolls horizontally.
 *
 * @ingroup widgets
 * @see scroll_widget
 * window gets a signal to resize to its preferred size.
 */
using horizontal_scroll_widget = scroll_widget<axis::horizontal>;

}} // namespace hi::v1
