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
#include "../layout/grid_layout.hpp"

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
    using horizontal_scroll_bar_type = scroll_bar_widget<axis::horizontal>;
    using vertical_scroll_bar_type = scroll_bar_widget<axis::vertical>;

    static constexpr hi::axis axis = Axis;

    observer<int> minimum_width = box_constraints::max_int();
    observer<int> minimum_height = box_constraints::max_int();

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

        auto unique_aperture = std::make_unique<scroll_aperture_widget>(this, minimum_width, minimum_height);
        auto unique_horizontal_scroll_bar = std::make_unique<horizontal_scroll_bar_type>(
            this, unique_aperture->content_width, unique_aperture->aperture_width, unique_aperture->offset_x);
        auto unique_vertical_scroll_bar = std::make_unique<vertical_scroll_bar_type>(
            this, unique_aperture->content_height, unique_aperture->aperture_height, unique_aperture->offset_y);

        if (to_bool(axis == axis::horizontal)) {
            minimum_width = 0;
        } else {
            unique_horizontal_scroll_bar->mode = widget_mode::collapse;
        }

        if (to_bool(axis == axis::vertical)) {
            minimum_height = 0;
        } else {
            unique_vertical_scroll_bar->mode = widget_mode::collapse;
        }

        _aperture = unique_aperture.get();
        _horizontal_scroll_bar = unique_horizontal_scroll_bar.get();
        _vertical_scroll_bar = unique_vertical_scroll_bar.get();

        _grid.add_cell(0, 0, std::move(unique_aperture));
        _grid.add_cell(1, 0, std::move(unique_vertical_scroll_bar));
        _grid.add_cell(0, 1, std::move(unique_horizontal_scroll_bar));
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
        co_yield _aperture;
        co_yield _vertical_scroll_bar;
        co_yield _horizontal_scroll_bar;
    }

    box_constraints const& set_constraints(set_constraints_context const& context) noexcept override
    {
        _layout = {};

        for (auto& cell : _grid) {
            cell.set_constraints(cell.value->set_constraints(context));
        }

        return _constraints = _grid.get_constraints(context.left_to_right());
    }

    void set_layout(widget_layout const& context) noexcept override
    {
        if (compare_store(_layout, context)) {
            _grid.set_layout(context.width(), context.height(), context.left_to_right(), context.theme->baseline_adjustment);
        }
        for (hilet& cell : _grid) {
            cell.value->set_layout(context.transform(cell.shape, 0.0f));
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
    grid_layout<std::unique_ptr<widget>> _grid;

    scroll_aperture_widget *_aperture;
    horizontal_scroll_bar_type *_horizontal_scroll_bar;
    vertical_scroll_bar_type *_vertical_scroll_bar;
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
