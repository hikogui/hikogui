// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "widget.hpp"
#include "scroll_bar_widget.hpp"
#include "scroll_delegate.hpp"
#include "../GUI/gui_window.hpp"
#include "../geometry/axis.hpp"

namespace tt {

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

        _layout_callback = std::make_shared<std::function<void()>>([this]() {
            _request_layout = true;
        });

        _scroll_content_width.subscribe(_layout_callback);
        _scroll_content_height.subscribe(_layout_callback);
        _scroll_aperture_width.subscribe(_layout_callback);
        _scroll_aperture_height.subscribe(_layout_callback);
        _scroll_offset_x.subscribe(_layout_callback);
        _scroll_offset_y.subscribe(_layout_callback);
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

        auto &widget = super::make_widget<Widget>(std::forward<Args>(args)...);
        _content = &widget;
        return widget;
    }

    /// @privatesection
    void init() noexcept override
    {
        super::init();

        _horizontal_scroll_bar =
            &super::make_widget<horizontal_scroll_bar_widget>(_scroll_content_width, _scroll_aperture_width, _scroll_offset_x);
        _vertical_scroll_bar =
            &super::make_widget<vertical_scroll_bar_widget>(_scroll_content_height, _scroll_aperture_height, _scroll_offset_y);

        if (auto delegate = _delegate.lock()) {
            delegate->init(*this);
        }
    }

    void deinit() noexcept override
    {
        if (auto delegate = _delegate.lock()) {
            delegate->deinit(*this);
        }
        super::deinit();
    }

    [[nodiscard]] float margin() const noexcept override
    {
        return 0.0f;
    }

    [[nodiscard]] bool constrain(utc_nanoseconds display_time_point, bool need_reconstrain) noexcept override
    {
        tt_axiom(is_gui_thread());
        tt_axiom(_content);

        auto has_updated_contraints = super::constrain(display_time_point, need_reconstrain);

        // Recurse into the selected widget.
        if (has_updated_contraints) {
            // As the widget will always add scrollbars if needed the minimum size is dictated to
            // the size of the scrollbars.
            _minimum_size = _content->minimum_size();
            _preferred_size = _content->preferred_size();
            _maximum_size = _content->maximum_size();

            // When there are scrollbars the minimum size is the minimum length of the scrollbar.
            // The maximum size is the minimum size of the content.
            if constexpr (any(axis & axis::horizontal)) {
                // The content could be smaller than the scrollbar.
                _minimum_size.width() = _horizontal_scroll_bar->minimum_size().width();
                _preferred_size.width() = std::max(_preferred_size.width(), _horizontal_scroll_bar->minimum_size().width());
                _maximum_size.width() = std::max(_maximum_size.width(), _horizontal_scroll_bar->minimum_size().width());
            }
            if constexpr (any(axis & axis::vertical)) {
                _minimum_size.height() = _vertical_scroll_bar->minimum_size().height();
                _preferred_size.height() = std::max(_preferred_size.height(), _vertical_scroll_bar->minimum_size().height());
                _maximum_size.height() = std::max(_maximum_size.height(), _vertical_scroll_bar->minimum_size().height());
            }

            // Make room for the scroll bars.
            if constexpr (any(axis & axis::horizontal)) {
                _minimum_size.height() += _horizontal_scroll_bar->preferred_size().height();
                _preferred_size.height() += _horizontal_scroll_bar->preferred_size().height();
                _maximum_size.height() += _horizontal_scroll_bar->preferred_size().height();
            }
            if constexpr (any(axis & axis::vertical)) {
                _minimum_size.width() += _vertical_scroll_bar->preferred_size().width();
                _preferred_size.width() += _vertical_scroll_bar->preferred_size().width();
                _maximum_size.width() += _vertical_scroll_bar->preferred_size().width();
            }
        }
        tt_axiom(_minimum_size <= _preferred_size && _preferred_size <= _maximum_size);
        return has_updated_contraints;
    }

    [[nodiscard]] void layout(utc_nanoseconds display_time_point, bool need_layout) noexcept override
    {
        tt_axiom(is_gui_thread());
        tt_axiom(_content);

        need_layout |= _request_layout.exchange(false);
        if (need_layout) {
            ttlet vertical_scroll_bar_width = _vertical_scroll_bar->preferred_size().width();
            ttlet horizontal_scroll_bar_height = _horizontal_scroll_bar->preferred_size().height();

            std::tie(_horizontal_scroll_bar->visible, _vertical_scroll_bar->visible) = needed_scrollbars();

            ttlet height_adjustment = _horizontal_scroll_bar->visible ? horizontal_scroll_bar_height : 0.0f;
            ttlet width_adjustment = _vertical_scroll_bar->visible ? vertical_scroll_bar_width : 0.0f;

            ttlet vertical_scroll_bar_rectangle = aarectangle{
                width() - vertical_scroll_bar_width, height_adjustment, vertical_scroll_bar_width, height() - height_adjustment};

            ttlet horizontal_scroll_bar_rectangle =
                aarectangle{0.0f, 0.0f, width() - width_adjustment, horizontal_scroll_bar_height};

            _vertical_scroll_bar->set_layout_parameters_from_parent(vertical_scroll_bar_rectangle);
            _horizontal_scroll_bar->set_layout_parameters_from_parent(horizontal_scroll_bar_rectangle);

            _aperture_rectangle = aarectangle{0.0f, height_adjustment, width() - width_adjustment, height() - height_adjustment};

            // We use the preferred size of the content for determining what to scroll.
            // This means it is possible for the scroll_content_width or scroll_content_height to be smaller
            // than the aperture.
            _scroll_content_width = _content->preferred_size().width();
            _scroll_content_height = _content->preferred_size().height();
            _scroll_aperture_width = _aperture_rectangle.width();
            _scroll_aperture_height = _aperture_rectangle.height();

            ttlet scroll_offset_x_max = std::max(_scroll_content_width - _scroll_aperture_width, 0.0f);
            ttlet scroll_offset_y_max = std::max(_scroll_content_height - _scroll_aperture_height, 0.0f);

            _scroll_offset_x = std::clamp(std::round(*_scroll_offset_x), 0.0f, scroll_offset_x_max);
            _scroll_offset_y = std::clamp(std::round(*_scroll_offset_y), 0.0f, scroll_offset_y_max);

            // Its size scroll content size, or the size of the aperture whichever is bigger.
            ttlet content_size = extent2{
                std::max(*_scroll_content_width, _aperture_rectangle.width()),
                std::max(*_scroll_content_height, _aperture_rectangle.height())};

            // The position of the content rectangle relative to the scroll view.
            // The size is further adjusted if the either the horizontal or vertical scroll bar is invisible.
            ttlet content_rectangle = aarectangle{
                -_scroll_offset_x, -_scroll_offset_y - height_adjustment, content_size.width(), content_size.height()};

            // Make a clipping rectangle that fits the aperture_rectangle exactly.
            _content->set_layout_parameters_from_parent(
                content_rectangle, _aperture_rectangle, _content->draw_layer - draw_layer);

            if constexpr (controls_window) {
                window.set_resize_border_priority(
                    true, not _vertical_scroll_bar->visible, not _horizontal_scroll_bar->visible, true);
            }
        }

        super::layout(display_time_point, need_layout);
    }

    [[nodiscard]] hitbox hitbox_test(point2 position) const noexcept override
    {
        tt_axiom(is_gui_thread());
        tt_axiom(_content);

        auto r = super::hitbox_test(position);

        if (_visible_rectangle.contains(position)) {
            // Claim mouse events for scrolling.
            r = std::max(r, hitbox{this, draw_layer});
        }

        return r;
    }

    bool handle_event(mouse_event const &event) noexcept override
    {
        tt_axiom(is_gui_thread());
        auto handled = super::handle_event(event);

        if (event.type == mouse_event::Type::Wheel) {
            handled = true;
            _scroll_offset_x += event.wheelDelta.x();
            _scroll_offset_y += event.wheelDelta.y();
            _request_layout = true;
            return true;
        }
        return handled;
    }

    void scroll_to_show(tt::rectangle rectangle) noexcept override
    {
        auto rectangle_ = bounding_rectangle(rectangle);

        float delta_x = 0.0f;
        if (rectangle_.right() > _aperture_rectangle.right()) {
            delta_x = rectangle_.right() - _aperture_rectangle.right();
        } else if (rectangle_.left() < _aperture_rectangle.left()) {
            delta_x = rectangle_.left() - _aperture_rectangle.left();
        }

        float delta_y = 0.0f;
        if (rectangle_.top() > _aperture_rectangle.top()) {
            delta_y = rectangle_.top() - _aperture_rectangle.top();
        } else if (rectangle_.bottom() < _aperture_rectangle.bottom()) {
            delta_y = rectangle_.bottom() - _aperture_rectangle.bottom();
        }

        _scroll_offset_x += delta_x;
        _scroll_offset_y += delta_y;

        // There may be recursive scroll view, and they all need to move until the rectangle is visible.
        if (parent) {
            parent->scroll_to_show(_local_to_parent * translate2(delta_x, delta_y) * rectangle);
        }
    }
    // @endprivatesection
private:
    std::weak_ptr<delegate_type> _delegate;
    widget *_content = nullptr;
    horizontal_scroll_bar_widget *_horizontal_scroll_bar = nullptr;
    vertical_scroll_bar_widget *_vertical_scroll_bar = nullptr;

    observable<float> _scroll_content_width;
    observable<float> _scroll_content_height;
    observable<float> _scroll_aperture_width;
    observable<float> _scroll_aperture_height;
    observable<float> _scroll_offset_x;
    observable<float> _scroll_offset_y;
    observable<float>::callback_ptr_type _layout_callback;

    aarectangle _aperture_rectangle;

    /** Calculate which scrollbars are needed to display the content.
     * @return has_horizontal_scroll_baar, has_vertical_scroll_bar
     */
    [[nodiscard]] std::pair<bool, bool> needed_scrollbars() const noexcept
    {
        ttlet content_size = _content->preferred_size();

        if (content_size <= size()) {
            return {false, false};
        } else if (content_size.width() - _vertical_scroll_bar->preferred_size().width() <= width()) {
            return {false, true};
        } else if (content_size.height() - _horizontal_scroll_bar->preferred_size().height() <= height()) {
            return {true, false};
        } else {
            return {true, true};
        }
    }
};

template<bool ControlsWindow = false>
using vertical_scroll_widget = scroll_widget<axis::vertical, ControlsWindow>;

template<bool ControlsWindow = false>
using horizontal_scroll_widget = scroll_widget<axis::horizontal, ControlsWindow>;

} // namespace tt