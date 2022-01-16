// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "widget.hpp"
#include "scroll_bar_widget.hpp"
#include "scroll_delegate.hpp"
#include "../GUI/gui_window.hpp"
#include "../geometry/axis.hpp"

namespace tt::inline v1 {

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
 * @tparam Axis the axis that the content may be scrolled. Allowed values are
 *              `axis::horizontal`, `axis::vertical` or `axis::both`.
 * @tparam ControlsWindow If set to true, when the content changes size the
 *                        window gets a signal to resize to its preferred size.
 */
template<axis Axis = axis::both, bool ControlsWindow = false>
class scroll_widget final : public widget {
public:
    static_assert(Axis == axis::horizontal or Axis == axis::vertical or Axis == axis::both);

    using super = widget;
    using delegate_type = scroll_delegate<Axis, ControlsWindow>;

    static constexpr tt::axis axis = Axis;
    static constexpr bool controls_window = ControlsWindow;

    ~scroll_widget()
    {
        if (auto delegate = _delegate.lock()) {
            delegate->deinit(*this);
        }
    }

    /** Constructs an empty scroll widget.
     *
     * @param window The window.
     * @param parent The parent widget.
     * @param delegate An optional delegate can be used to populate the scroll widget
     *                 during initialization.
     */
    scroll_widget(gui_window &window, widget *parent, std::weak_ptr<delegate_type> delegate = {}) noexcept :
        super(window, parent), _delegate(std::move(delegate))
    {
        tt_axiom(is_gui_thread());

        if (parent) {
            // The tab-widget will not draw itself, only its selected content.
            semantic_layer = parent->semantic_layer;
        }

        _scroll_content_width.subscribe(_relayout_callback);
        _scroll_content_height.subscribe(_relayout_callback);
        _scroll_aperture_width.subscribe(_relayout_callback);
        _scroll_aperture_height.subscribe(_relayout_callback);
        _scroll_offset_x.subscribe(_relayout_callback);
        _scroll_offset_y.subscribe(_relayout_callback);

        _horizontal_scroll_bar = std::make_unique<horizontal_scroll_bar_widget>(
            window, this, _scroll_content_width, _scroll_aperture_width, _scroll_offset_x);
        _vertical_scroll_bar = std::make_unique<vertical_scroll_bar_widget>(
            window, this, _scroll_content_height, _scroll_aperture_height, _scroll_offset_y);

        if (auto d = _delegate.lock()) {
            d->init(*this);
        }
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
    Widget &make_widget(Args &&...args) noexcept
    {
        tt_axiom(is_gui_thread());
        tt_axiom(not _content);

        auto tmp = std::make_unique<Widget>(window, this, std::forward<Args>(args)...);
        auto &ref = *tmp;
        _content = std::move(tmp);
        return ref;
    }

    /// @privatesection
    [[nodiscard]] pmr::generator<widget *> children(std::pmr::polymorphic_allocator<> &) const noexcept override
    {
        co_yield _content.get();
        co_yield _vertical_scroll_bar.get();
        co_yield _horizontal_scroll_bar.get();
    }

    widget_constraints const &set_constraints() noexcept override
    {
        tt_axiom(_content);

        _layout = {};
        ttlet content_constraints = _content->set_constraints();
        ttlet horizontal_constraints = _horizontal_scroll_bar->set_constraints();
        ttlet vertical_constraints = _vertical_scroll_bar->set_constraints();

        // The scroll constraints are the constraints of the content including the margins of the content.
        _scroll_margins = content_constraints.margins;
        _scroll_constraints = content_constraints +
            extent2{_scroll_margins.left() + _scroll_margins.right(), _scroll_margins.bottom() + _scroll_margins.top()};
        _scroll_constraints.margins = {};

        // Scrollbars are the same size as the content plus their margins. Therefor the scroll view has zero margins itself.
        _constraints = _scroll_constraints;

        // When there are scrollbars the widget size is at least the minimum length of the scrollbar.
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

        // Make room for the thickness of the scroll bars.
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

    void set_layout(widget_layout const &layout) noexcept override
    {
        if (compare_store(_layout, layout)) {
            ttlet vertical_scroll_bar_width = _vertical_scroll_bar->constraints().preferred.width();
            ttlet horizontal_scroll_bar_height = _horizontal_scroll_bar->constraints().preferred.height();

            std::tie(_horizontal_scroll_bar->visible, _vertical_scroll_bar->visible) = needed_scrollbars();

            _height_adjustment = _horizontal_scroll_bar->visible ? horizontal_scroll_bar_height : 0.0f;
            _width_adjustment = _vertical_scroll_bar->visible ? vertical_scroll_bar_width : 0.0f;

            _vertical_scroll_bar_rectangle = aarectangle{
                layout.width() - vertical_scroll_bar_width,
                _height_adjustment,
                vertical_scroll_bar_width,
                layout.height() - _height_adjustment};

            _horizontal_scroll_bar_rectangle =
                aarectangle{0.0f, 0.0f, layout.width() - _width_adjustment, horizontal_scroll_bar_height};

            _aperture_rectangle =
                aarectangle{0.0f, _height_adjustment, layout.width() - _width_adjustment, layout.height() - _height_adjustment};

            // We use the preferred size of the content for determining what to scroll.
            // This means it is possible for the scroll_content_width or scroll_content_height to be smaller
            // than the aperture.
            _scroll_content_width = _scroll_constraints.preferred.width();
            _scroll_content_height = _scroll_constraints.preferred.height();
            _scroll_aperture_width = _aperture_rectangle.width();
            _scroll_aperture_height = _aperture_rectangle.height();

            if constexpr (controls_window) {
                window.set_resize_border_priority(
                    true, not _vertical_scroll_bar->visible, not _horizontal_scroll_bar->visible, true);
            }
        }

        if (_vertical_scroll_bar->visible) {
            _vertical_scroll_bar->set_layout(layout.transform(_vertical_scroll_bar_rectangle));
        }
        if (_horizontal_scroll_bar->visible) {
            _horizontal_scroll_bar->set_layout(layout.transform(_horizontal_scroll_bar_rectangle));
        }

        ttlet scroll_offset_x_max = std::max(_scroll_content_width - _scroll_aperture_width, 0.0f);
        ttlet scroll_offset_y_max = std::max(_scroll_content_height - _scroll_aperture_height, 0.0f);
        _scroll_offset_x = std::clamp(std::round(*_scroll_offset_x), 0.0f, scroll_offset_x_max);
        _scroll_offset_y = std::clamp(std::round(*_scroll_offset_y), 0.0f, scroll_offset_y_max);

        // Its size scroll content size, or the size of the aperture whichever is bigger.
        ttlet content_size = extent2{
            std::max(*_scroll_content_width, _aperture_rectangle.width()) - _scroll_margins.left() - _scroll_margins.right(),
            std::max(*_scroll_content_height, _aperture_rectangle.height()) - _scroll_margins.bottom() - _scroll_margins.top()};

        // The position of the content rectangle relative to the scroll view.
        // The size is further adjusted if the either the horizontal or vertical scroll bar is invisible.
        _content_rectangle = {
            -_scroll_offset_x + _scroll_margins.left(),
            -_scroll_offset_y + _height_adjustment + _scroll_margins.bottom(),
            content_size.width(),
            content_size.height()};

        // The scrollbars and content need to be at a higher elevation, so that hitbox check
        // will work correctly for handling scrolling with mouse wheel.
        _content->set_layout(layout.transform(_content_rectangle, 1.0f, _aperture_rectangle));
    }

    void draw(draw_context const &context) noexcept
    {
        if (visible) {
            _vertical_scroll_bar->draw(context);
            _horizontal_scroll_bar->draw(context);
            _content->draw(context);
        }
    }

    [[nodiscard]] hitbox hitbox_test(point3 position) const noexcept override
    {
        tt_axiom(is_gui_thread());

        if (visible and enabled) {
            auto r = _content->hitbox_test_from_parent(position);
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

    bool handle_event(mouse_event const &event) noexcept override
    {
        tt_axiom(is_gui_thread());
        auto handled = super::handle_event(event);

        if (event.type == mouse_event::Type::Wheel) {
            handled = true;
            ttlet new_scroll_offset_x = _scroll_offset_x + event.wheelDelta.x();
            ttlet new_scroll_offset_y = _scroll_offset_y + event.wheelDelta.y();
            ttlet max_scroll_offset_x = std::max(0.0f, _scroll_content_width - _scroll_aperture_width);
            ttlet max_scroll_offset_y = std::max(0.0f, _scroll_content_height - _scroll_aperture_height);

            _scroll_offset_x = std::clamp(new_scroll_offset_x, 0.0f, max_scroll_offset_x);
            _scroll_offset_y = std::clamp(new_scroll_offset_y, 0.0f, max_scroll_offset_y);
            request_relayout();
            return true;
        }
        return handled;
    }

    void scroll_to_show(tt::aarectangle to_show) noexcept override
    {
        ttlet window_clipping_rectangle = layout().window_clipping_rectangle();

        float delta_x = 0.0f;
        if (to_show.right() > window_clipping_rectangle.right()) {
            delta_x = to_show.right() - window_clipping_rectangle.right();
        } else if (to_show.left() < window_clipping_rectangle.left()) {
            delta_x = to_show.left() - window_clipping_rectangle.left();
        }

        float delta_y = 0.0f;
        if (to_show.top() > window_clipping_rectangle.top()) {
            delta_y = to_show.top() - window_clipping_rectangle.top();
        } else if (to_show.bottom() < window_clipping_rectangle.bottom()) {
            delta_y = to_show.bottom() - window_clipping_rectangle.bottom();
        }

        _scroll_offset_x += delta_x;
        _scroll_offset_y += delta_y;

        // There may be recursive scroll view, and they all need to move until the rectangle is visible.
        if (parent) {
            parent->scroll_to_show(translate2(delta_x, delta_y) * to_show);
        }
    }
    // @endprivatesection
private:
    std::weak_ptr<delegate_type> _delegate;

    aarectangle _aperture_rectangle;
    aarectangle _content_rectangle;
    std::unique_ptr<widget> _content;

    margins _scroll_margins;
    widget_constraints _scroll_constraints;

    float _height_adjustment;
    aarectangle _horizontal_scroll_bar_rectangle;
    std::unique_ptr<horizontal_scroll_bar_widget> _horizontal_scroll_bar;

    float _width_adjustment;
    aarectangle _vertical_scroll_bar_rectangle;
    std::unique_ptr<vertical_scroll_bar_widget> _vertical_scroll_bar;

    observable<float> _scroll_content_width;
    observable<float> _scroll_content_height;
    observable<float> _scroll_aperture_width;
    observable<float> _scroll_aperture_height;
    observable<float> _scroll_offset_x;
    observable<float> _scroll_offset_y;

    /** Calculate which scrollbars are needed to display the content.
     * @return has_horizontal_scroll_bar, has_vertical_scroll_bar
     */
    [[nodiscard]] std::pair<bool, bool> needed_scrollbars() const noexcept
    {
        ttlet has_horizontal_scroll_bar =
            layout().width() - _vertical_scroll_bar->constraints().preferred.width() < _scroll_constraints.preferred.width();
        ttlet has_vertical_scroll_bar =
            layout().height() - _horizontal_scroll_bar->constraints().preferred.height() < _scroll_constraints.preferred.height();

        return {has_horizontal_scroll_bar, has_vertical_scroll_bar};
    }
};

template<bool ControlsWindow = false>
using vertical_scroll_widget = scroll_widget<axis::vertical, ControlsWindow>;

template<bool ControlsWindow = false>
using horizontal_scroll_widget = scroll_widget<axis::horizontal, ControlsWindow>;

} // namespace tt::inline v1
