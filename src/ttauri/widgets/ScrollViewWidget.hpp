// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "widget.hpp"
#include "GridLayoutWidget.hpp"
#include "ScrollBarWidget.hpp"

namespace tt {

template<bool CanScrollHorizontally = true, bool CanScrollVertically = true, bool ControlsWindow = true>
class ScrollViewWidget final : public widget {
public:
    using super = widget;

    static constexpr bool can_scroll_horizontally = CanScrollHorizontally;
    static constexpr bool can_scroll_vertically = CanScrollVertically;
    static constexpr bool controls_window = ControlsWindow;

    ScrollViewWidget(Window &window, std::shared_ptr<widget> parent) noexcept : super(window, parent)
    {
        if (parent) {
            // The tab-widget will not draw itself, only its selected content.
            ttlet lock = std::scoped_lock(gui_system_mutex);
            _semantic_layer = parent->semantic_layer();
        }
        _margin = 0.0f;
    }

    ~ScrollViewWidget() {}

    void initialize() noexcept override
    {
        if constexpr (can_scroll_horizontally) {
            horizontal_scroll_bar = std::make_shared<ScrollBarWidget<false>>(
                window, shared_from_this(), scroll_content_width, scroll_aperture_width, scroll_offset_x);
            horizontal_scroll_bar->initialize();
        }
        if constexpr (can_scroll_vertically) {
            vertical_scroll_bar = std::make_shared<ScrollBarWidget<true>>(
                window, shared_from_this(), scroll_content_height, scroll_aperture_height, scroll_offset_y);
            vertical_scroll_bar->initialize();
        }
    }

    [[nodiscard]] bool update_constraints(hires_utc_clock::time_point display_time_point, bool need_reconstrain) noexcept override
    {
        tt_assume(gui_system_mutex.recurse_lock_count());
        tt_assume(content);
        tt_assume(!can_scroll_horizontally || horizontal_scroll_bar);
        tt_assume(!can_scroll_vertically || vertical_scroll_bar);

        auto has_updated_contraints = super::update_constraints(display_time_point, need_reconstrain);
        has_updated_contraints |= content->update_constraints(display_time_point, need_reconstrain);
        if constexpr (can_scroll_horizontally) {
            has_updated_contraints |= horizontal_scroll_bar->update_constraints(display_time_point, need_reconstrain);
        }
        if constexpr (can_scroll_vertically) {
            has_updated_contraints |= vertical_scroll_bar->update_constraints(display_time_point, need_reconstrain);
        }

        // Recurse into the selected widget.
        if (has_updated_contraints) {
            auto width = content->preferred_size().width();
            auto height = content->preferred_size().height();

            // When there are scrollbars the minimum size is the minimum length of the scrollbar.
            // The maximum size is the minimum size of the content.
            if constexpr (can_scroll_horizontally) {
                // The content could be smaller than the scrollbar.
                ttlet minimum_width = std::min(width.minimum(), horizontal_scroll_bar->preferred_size().width().minimum());
                width = {minimum_width, width.minimum()};
            }
            if constexpr (can_scroll_vertically) {
                ttlet minimum_height = std::min(height.minimum(), vertical_scroll_bar->preferred_size().height().minimum());
                height = {minimum_height, height.minimum()};
            }

            // Make room for the scroll bars.
            if constexpr (can_scroll_horizontally) {
                height += horizontal_scroll_bar->preferred_size().height();
            }
            if constexpr (can_scroll_vertically) {
                width += vertical_scroll_bar->preferred_size().width();
            }

            _preferred_size = interval_vec2{width, height};
            _preferred_base_line = {};
        }

        return has_updated_contraints;
    }

    [[nodiscard]] bool update_layout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept override
    {
        tt_assume(gui_system_mutex.recurse_lock_count());
        tt_assume(content);

        auto need_redraw = need_layout |= std::exchange(_request_relayout, false);
        if (need_layout) {
            // Calculate the width and height of the scroll-bars, make the infinity thin when they don't exist.
            ttlet vertical_scroll_bar_width =
                can_scroll_vertically ? vertical_scroll_bar->preferred_size().minimum().width() : 0.0f;
            ttlet horizontal_scroll_bar_height =
                can_scroll_horizontally ? horizontal_scroll_bar->preferred_size().minimum().height() : 0.0f;
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
                horizontal_scroll_bar->set_layout_parameters(
                    mat::T2{_window_rectangle} * horizontal_scroll_bar_rectangle, _window_clipping_rectangle);
            }
            if constexpr (can_scroll_vertically) {
                vertical_scroll_bar->set_layout_parameters(
                    mat::T2{_window_rectangle} * vertical_scroll_bar_rectangle, _window_clipping_rectangle);
            }

            auto aperture_x = rectangle().x();
            auto aperture_y = horizontal_scroll_bar_rectangle.top();
            auto aperture_width = horizontal_scroll_bar_rectangle.width();
            auto aperture_height = vertical_scroll_bar_rectangle.height();

            // We can not use the content_rectangle is the window for the content.
            // We need to calculate the window_content_rectangle, to positions the content after scrolling.
            scroll_content_width = can_scroll_horizontally ? content->preferred_size().minimum().width() : aperture_width;
            scroll_content_height = can_scroll_vertically ? content->preferred_size().minimum().height() : aperture_height;

            scroll_aperture_width = aperture_width;
            scroll_aperture_height = aperture_height;

            ttlet scroll_offset_x_max = *scroll_content_width - aperture_width;
            ttlet scroll_offset_y_max = *scroll_content_height - aperture_height;

            scroll_offset_x = std::clamp(std::round(*scroll_offset_x), 0.0f, scroll_offset_x_max);
            scroll_offset_y = std::clamp(std::round(*scroll_offset_y), 0.0f, scroll_offset_y_max);

            auto content_x = -*scroll_offset_x;
            auto content_y = -*scroll_offset_y;
            auto content_width = *scroll_content_width;
            auto content_height = *scroll_content_height;

            // Visual hack, to extend the aperture over the invisible scrollbars.
            ttlet content_can_extent_vertically = content->preferred_size().maximum().height() >= rectangle().height();
            ttlet content_can_extent_horizontally = content->preferred_size().maximum().width() >= rectangle().width();

            if (can_scroll_horizontally && !horizontal_scroll_bar->visible() && content_can_extent_vertically) {
                ttlet delta_height = horizontal_scroll_bar_rectangle.height();
                aperture_height += delta_height;
                aperture_y -= delta_height;
                content_height += delta_height;
                content_y -= delta_height;
            }

            if (can_scroll_vertically && !vertical_scroll_bar->visible() && content_can_extent_horizontally) {
                ttlet delta_width = vertical_scroll_bar_rectangle.width();
                aperture_width += delta_width;
                content_width += delta_width;
            }

            if constexpr (controls_window) {
                ttlet has_horizontal_scroll_bar = can_scroll_horizontally && horizontal_scroll_bar->visible();
                ttlet has_vertical_scroll_bar = can_scroll_vertically && vertical_scroll_bar->visible();
                window.set_resize_border_priority(true, !has_vertical_scroll_bar, !has_horizontal_scroll_bar, true);
            }

            // Make a clipping rectangle that fits the content_rectangle exactly.
            ttlet aperture_rectangle = aarect{aperture_x, aperture_y, aperture_width, aperture_height};
            ttlet window_aperture_clipping_rectangle =
                intersect(_window_clipping_rectangle, mat::T2{_window_rectangle} * aperture_rectangle);

            ttlet content_rectangle = aarect{content_x, content_y, content_width, content_height};

            content->set_layout_parameters(mat::T2{_window_rectangle} * content_rectangle, window_aperture_clipping_rectangle);
        }

        need_redraw |= content->update_layout(display_time_point, need_layout);
        if constexpr (can_scroll_horizontally) {
            need_redraw |= horizontal_scroll_bar->update_layout(display_time_point, need_layout);
        }
        if constexpr (can_scroll_vertically) {
            need_redraw |= vertical_scroll_bar->update_layout(display_time_point, need_layout);
        }

        return super::update_layout(display_time_point, need_layout) || need_redraw;
    }

    void draw(DrawContext context, hires_utc_clock::time_point display_time_point) noexcept override
    {
        tt_assume(gui_system_mutex.recurse_lock_count());
        tt_assume(content);

        if constexpr (can_scroll_horizontally) {
            horizontal_scroll_bar->draw(horizontal_scroll_bar->make_draw_context(context), display_time_point);
        }
        if constexpr (can_scroll_vertically) {
            vertical_scroll_bar->draw(vertical_scroll_bar->make_draw_context(context), display_time_point);
        }

        content->draw(content->make_draw_context(context), display_time_point);

        super::draw(std::move(context), display_time_point);
    }

    bool handle_command_recursive(command command, std::vector<std::shared_ptr<widget>> const &reject_list) noexcept override
    {
        tt_assume(gui_system_mutex.recurse_lock_count());

        auto handled = false;
        tt_assume(content->parent.lock().get() == this);
        handled |= content->handle_command_recursive(command, reject_list);
        if constexpr (can_scroll_horizontally) {
            tt_assume(horizontal_scroll_bar->parent.lock().get() == this);
            handled |= horizontal_scroll_bar->handle_command_recursive(command, reject_list);
        }
        if constexpr (can_scroll_vertically) {
            tt_assume(vertical_scroll_bar->parent.lock().get() == this);
            handled |= vertical_scroll_bar->handle_command_recursive(command, reject_list);
        }

        handled |= super::handle_command_recursive(command, reject_list);
        return handled;
    }

    [[nodiscard]] HitBox hitbox_test(vec window_position) const noexcept override
    {
        ttlet lock = std::scoped_lock(gui_system_mutex);
        tt_assume(content);

        auto r = HitBox{};

        if (_window_clipping_rectangle.contains(window_position)) {
            // Claim mouse events for scrolling.
            r = std::max(r, HitBox{weak_from_this(), _draw_layer});
        }

        r = std::max(r, content->hitbox_test(window_position));
        if constexpr (can_scroll_horizontally) {
            r = std::max(r, horizontal_scroll_bar->hitbox_test(window_position));
        }
        if constexpr (can_scroll_vertically) {
            r = std::max(r, vertical_scroll_bar->hitbox_test(window_position));
        }
        return r;
    }

    std::shared_ptr<widget> next_keyboard_widget(std::shared_ptr<widget> const &currentKeyboardWidget, bool reverse) const noexcept
    {
        ttlet lock = std::scoped_lock(gui_system_mutex);
        tt_assume(content);

        // Scrollbars are never keyboard focus targets.
        return content->next_keyboard_widget(currentKeyboardWidget, reverse);
    }

    template<typename WidgetType = GridLayoutWidget, typename... Args>
    std::shared_ptr<WidgetType> make_widget(Args const &... args) noexcept
    {
        ttlet lock = std::scoped_lock(gui_system_mutex);

        auto widget = std::make_shared<WidgetType>(window, shared_from_this(), args...);
        widget->initialize();

        content = widget;
        _request_reconstrain = true;
        return widget;
    }

    bool handle_mouse_event(MouseEvent const &event) noexcept override
    {
        ttlet lock = std::scoped_lock(gui_system_mutex);
        auto handled = super::handle_mouse_event(event);

        if (event.type == MouseEvent::Type::Wheel) {
            handled = true;
            scroll_offset_x += event.wheelDelta.x();
            scroll_offset_y += event.wheelDelta.y();
            _request_relayout = true;
            return true;
        }
        return handled;
    }

private:
    std::shared_ptr<widget> content;
    std::shared_ptr<ScrollBarWidget<false>> horizontal_scroll_bar;
    std::shared_ptr<ScrollBarWidget<true>> vertical_scroll_bar;

    observable<float> scroll_content_width;
    observable<float> scroll_content_height;
    observable<float> scroll_aperture_width;
    observable<float> scroll_aperture_height;
    observable<float> scroll_offset_x;
    observable<float> scroll_offset_y;
};

template<bool ControlsWindow = false>
using VerticalScrollViewWidget = ScrollViewWidget<false, true, ControlsWindow>;

template<bool ControlsWindow = false>
using HorizontalScrollViewWidget = ScrollViewWidget<true, false, ControlsWindow>;

} // namespace tt