// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "abstract_container_widget.hpp"
#include "grid_layout_widget.hpp"
#include "scroll_bar_widget.hpp"

namespace tt {

template<bool CanScrollHorizontally = true, bool CanScrollVertically = true, bool ControlsWindow = true>
class scroll_view_widget final : public abstract_container_widget {
public:
    using super = abstract_container_widget;

    static constexpr bool can_scroll_horizontally = CanScrollHorizontally;
    static constexpr bool can_scroll_vertically = CanScrollVertically;
    static constexpr bool controls_window = ControlsWindow;

    scroll_view_widget(gui_window &window, std::shared_ptr<abstract_container_widget> parent) noexcept : super(window, parent)
    {
        if (parent) {
            // The tab-widget will not draw itself, only its selected content.
            ttlet lock = std::scoped_lock(gui_system_mutex);
            _semantic_layer = parent->semantic_layer();
        }
        _margin = 0.0f;
    }

    ~scroll_view_widget() {}

    void init() noexcept override
    {
        if constexpr (can_scroll_horizontally) {
            _horizontal_scroll_bar =
                super::make_widget<scroll_bar_widget<false>>(_scroll_content_width, _scroll_aperture_width, _scroll_offset_x);
        }
        if constexpr (can_scroll_vertically) {
            _vertical_scroll_bar =
                super::make_widget<scroll_bar_widget<true>>(_scroll_content_height, _scroll_aperture_height, _scroll_offset_y);
        }
    }

    [[nodiscard]] bool update_constraints(hires_utc_clock::time_point display_time_point, bool need_reconstrain) noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());
        tt_axiom(_content);
        tt_axiom(!can_scroll_horizontally || _horizontal_scroll_bar);
        tt_axiom(!can_scroll_vertically || _vertical_scroll_bar);

        auto has_updated_contraints = super::update_constraints(display_time_point, need_reconstrain);

        // Recurse into the selected widget.
        if (has_updated_contraints) {
            auto width = _content->preferred_size().width();
            auto height = _content->preferred_size().height();

            // When there are scrollbars the minimum size is the minimum length of the scrollbar.
            // The maximum size is the minimum size of the content.
            if constexpr (can_scroll_horizontally) {
                // The content could be smaller than the scrollbar.
                ttlet minimum_width = std::min(width.minimum(), _horizontal_scroll_bar->preferred_size().width().minimum());
                width = {minimum_width, width.minimum()};
            }
            if constexpr (can_scroll_vertically) {
                ttlet minimum_height = std::min(height.minimum(), _vertical_scroll_bar->preferred_size().height().minimum());
                height = {minimum_height, height.minimum()};
            }

            // Make room for the scroll bars.
            if constexpr (can_scroll_horizontally) {
                height += _horizontal_scroll_bar->preferred_size().height();
            }
            if constexpr (can_scroll_vertically) {
                width += _vertical_scroll_bar->preferred_size().width();
            }

            _preferred_size = interval_extent2{width, height};
        }

        return has_updated_contraints;
    }

    [[nodiscard]] void update_layout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());
        tt_axiom(_content);

        need_layout |= std::exchange(_request_relayout, false);
        if (need_layout) {
            // Calculate the width and height of the scroll-bars, make the infinity thin when they don't exist.
            ttlet vertical_scroll_bar_width =
                can_scroll_vertically ? _vertical_scroll_bar->preferred_size().minimum().width() : 0.0f;
            ttlet horizontal_scroll_bar_height =
                can_scroll_horizontally ? _horizontal_scroll_bar->preferred_size().minimum().height() : 0.0f;
            ttlet vertical_scroll_bar_height = rectangle().height() - horizontal_scroll_bar_height;
            ttlet horizontal_scroll_bar_widht = rectangle().width() - vertical_scroll_bar_width;

            // Calculate the rectangles based on the sizes of the scrollbars.
            ttlet vertical_scroll_bar_rectangle = aarect{
                rectangle().right() - vertical_scroll_bar_width,
                rectangle().y() + horizontal_scroll_bar_height,
                vertical_scroll_bar_width,
                rectangle().height() - horizontal_scroll_bar_height};

            ttlet horizontal_scroll_bar_rectangle = aarect{
                rectangle().x(), rectangle().y(), rectangle().width() - vertical_scroll_bar_width, horizontal_scroll_bar_height};

            // Update layout parameters for both scrollbars.
            if constexpr (can_scroll_horizontally) {
                _horizontal_scroll_bar->set_layout_parameters_from_parent(horizontal_scroll_bar_rectangle);
            }
            if constexpr (can_scroll_vertically) {
                _vertical_scroll_bar->set_layout_parameters_from_parent(vertical_scroll_bar_rectangle);
            }

            auto aperture_x = rectangle().x();
            auto aperture_y = horizontal_scroll_bar_rectangle.top();
            auto aperture_width = horizontal_scroll_bar_rectangle.width();
            auto aperture_height = vertical_scroll_bar_rectangle.height();

            // We can not use the content_rectangle is the window for the content.
            // We need to calculate the window_content_rectangle, to positions the content after scrolling.
            _scroll_content_width = can_scroll_horizontally ? _content->preferred_size().minimum().width() : aperture_width;
            _scroll_content_height = can_scroll_vertically ? _content->preferred_size().minimum().height() : aperture_height;

            _scroll_aperture_width = aperture_width;
            _scroll_aperture_height = aperture_height;

            ttlet scroll_offset_x_max = std::max(*_scroll_content_width - aperture_width, 0.0f);
            ttlet scroll_offset_y_max = std::max(*_scroll_content_height - aperture_height, 0.0f);

            _scroll_offset_x = std::clamp(std::round(*_scroll_offset_x), 0.0f, scroll_offset_x_max);
            _scroll_offset_y = std::clamp(std::round(*_scroll_offset_y), 0.0f, scroll_offset_y_max);

            auto content_x = -*_scroll_offset_x;
            auto content_y = -*_scroll_offset_y;
            auto content_width = *_scroll_content_width;
            auto content_height = *_scroll_content_height;

            if (can_scroll_horizontally && !_horizontal_scroll_bar->visible()) {
                ttlet delta_height = horizontal_scroll_bar_rectangle.height();
                aperture_height += delta_height;
                aperture_y -= delta_height;
                content_height += delta_height;
                content_y -= delta_height;
            }

            if (can_scroll_vertically && !_vertical_scroll_bar->visible()) {
                ttlet delta_width = vertical_scroll_bar_rectangle.width();
                aperture_width += delta_width;
                content_width += delta_width;
            }

            if constexpr (controls_window) {
                ttlet has_horizontal_scroll_bar = can_scroll_horizontally && _horizontal_scroll_bar->visible();
                ttlet has_vertical_scroll_bar = can_scroll_vertically && _vertical_scroll_bar->visible();
                window.set_resize_border_priority(true, !has_vertical_scroll_bar, !has_horizontal_scroll_bar, true);
            }

            // Make a clipping rectangle that fits the content_rectangle exactly.
            ttlet aperture_rectangle = aarect{aperture_x, aperture_y, aperture_width, aperture_height};
            ttlet content_rectangle = aarect{content_x, content_y, content_width, content_height};

            _content->set_layout_parameters_from_parent(
                content_rectangle, aperture_rectangle, _content->draw_layer() - _draw_layer);
        }

        super::update_layout(display_time_point, need_layout);
    }

    [[nodiscard]] hit_box hitbox_test(point2 position) const noexcept override
    {
        ttlet lock = std::scoped_lock(gui_system_mutex);
        tt_axiom(_content);

        auto r = super::hitbox_test(position);

        if (rectangle().contains(position)) {
            // Claim mouse events for scrolling.
            r = std::max(r, hit_box{weak_from_this(), _draw_layer});
        }

        return r;
    }

    template<typename WidgetType = grid_layout_widget, typename... Args>
    std::shared_ptr<WidgetType> make_widget(Args &&...args) noexcept
    {
        ttlet lock = std::scoped_lock(gui_system_mutex);

        auto widget = super::make_widget<WidgetType>(std::forward<Args>(args)...);
        tt_axiom(!_content);
        _content = widget;
        return widget;
    }

    bool handle_event(mouse_event const &event) noexcept override
    {
        ttlet lock = std::scoped_lock(gui_system_mutex);
        auto handled = super::handle_event(event);

        if (event.type == mouse_event::Type::Wheel) {
            handled = true;
            _scroll_offset_x += event.wheelDelta.x();
            _scroll_offset_y += event.wheelDelta.y();
            _request_relayout = true;
            return true;
        }
        return handled;
    }

private:
    std::shared_ptr<widget> _content;
    std::shared_ptr<scroll_bar_widget<false>> _horizontal_scroll_bar;
    std::shared_ptr<scroll_bar_widget<true>> _vertical_scroll_bar;

    observable<float> _scroll_content_width;
    observable<float> _scroll_content_height;
    observable<float> _scroll_aperture_width;
    observable<float> _scroll_aperture_height;
    observable<float> _scroll_offset_x;
    observable<float> _scroll_offset_y;
};

template<bool ControlsWindow = false>
using vertical_scroll_view_widget = scroll_view_widget<false, true, ControlsWindow>;

template<bool ControlsWindow = false>
using horizontal_scroll_view_widget = scroll_view_widget<true, false, ControlsWindow>;

} // namespace tt