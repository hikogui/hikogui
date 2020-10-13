// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "Widget.hpp"
#include "GridLayoutWidget.hpp"
#include "ScrollBarWidget.hpp"
#include "../cells/ImageCell.hpp"
#include "../cells/TextCell.hpp"

namespace tt {

template<bool CanScrollHorizontally = true, bool CanScrollVertically = true>
class ScrollViewWidget final : public Widget {
public:
    static constexpr bool can_scroll_horizontally = CanScrollHorizontally;
    static constexpr bool can_scroll_vertically = CanScrollVertically;

    ScrollViewWidget(Window &window, Widget *parent) noexcept : Widget(window, parent)
    {
        if (parent) {
            // The tab-widget will not draw itself, only its selected content.
            ttlet lock = std::scoped_lock(parent->mutex);
            _semantic_layer = parent->semantic_layer();
        }
        _margin = 0.0f;

        if constexpr (can_scroll_horizontally) {
            horizontal_scroll_bar = std::make_unique<ScrollBarWidget<false>>(
                window, this, scroll_content_width, scroll_aperture_width, scroll_offset_x);
        }
        if constexpr (can_scroll_vertically) {
            vertical_scroll_bar = std::make_unique<ScrollBarWidget<true>>(
                window, this, scroll_content_height, scroll_aperture_height, scroll_offset_y);
        }
    }

    ~ScrollViewWidget() {}

    [[nodiscard]] bool updateConstraints() noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());
        tt_assume(content);
        tt_assume(!can_scroll_horizontally || horizontal_scroll_bar);
        tt_assume(!can_scroll_vertically || vertical_scroll_bar);

        if (can_scroll_horizontally) {
            horizontal_scroll_bar->mutex.lock();
        }
        if (can_scroll_vertically) {
            vertical_scroll_bar->mutex.lock();
        }
        ttlet content_lock = std::scoped_lock{content->mutex};

        auto has_updated_contraints = Widget::updateConstraints();
        has_updated_contraints |= content->updateConstraints();
        if constexpr (can_scroll_horizontally) {
            has_updated_contraints |= horizontal_scroll_bar->updateConstraints();
        }
        if constexpr (can_scroll_vertically) {
            has_updated_contraints |= vertical_scroll_bar->updateConstraints();
        }

        // Recurse into the selected widget.
        if (has_updated_contraints) {
            auto width = content->preferred_size().width();
            auto height = content->preferred_size().height();

            // When there are scrollbars the minimum size is the minimum length of the scrollbar.
            // The maximum size is the minimum size of the content.
            if constexpr (can_scroll_horizontally) {
                width = {horizontal_scroll_bar->preferred_size().width().minimum(), width.minimum()};
            }
            if constexpr (can_scroll_vertically) {
                height = {vertical_scroll_bar->preferred_size().height().minimum(), height.minimum()};
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

        if (can_scroll_horizontally) {
            horizontal_scroll_bar->mutex.unlock();
        }
        if (can_scroll_vertically) {
            vertical_scroll_bar->mutex.unlock();
        }
        return has_updated_contraints;
    }

    [[nodiscard]] bool updateLayout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());
        tt_assume(content);

        if (can_scroll_horizontally) {
            horizontal_scroll_bar->mutex.lock();
        }
        if (can_scroll_vertically) {
            vertical_scroll_bar->mutex.lock();
        }
        ttlet content_lock = std::scoped_lock{content->mutex};

        auto need_redraw = need_layout |= std::exchange(requestLayout, false);
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

            ttlet aperture_rectangle = aarect{
                rectangle().x(),
                horizontal_scroll_bar_rectangle.top(),
                horizontal_scroll_bar_rectangle.width(),
                vertical_scroll_bar_rectangle.height()};

            // We can not use the content_rectangle is the window for the content.
            // We need to calculate the window_content_rectangle, to positions the content after scrolling.
            scroll_content_width =
                can_scroll_horizontally ? content->preferred_size().minimum().width() : aperture_rectangle.width();
            scroll_content_height =
                can_scroll_vertically ? content->preferred_size().minimum().height() : aperture_rectangle.height();

            scroll_aperture_width = aperture_rectangle.width();
            scroll_aperture_height = aperture_rectangle.height();

            ttlet scroll_offset_x_max = *scroll_content_width - *scroll_aperture_width;
            ttlet scroll_offset_y_max = *scroll_content_height - *scroll_aperture_height;

            scroll_offset_x = std::clamp(std::round(*scroll_offset_x), 0.0f, scroll_offset_x_max);
            scroll_offset_y = std::clamp(std::round(*scroll_offset_y), 0.0f, scroll_offset_y_max);

            ttlet content_rectangle = aarect{-*scroll_offset_x, -*scroll_offset_y, *scroll_content_width, *scroll_content_height};

            // Make a clipping rectangle that fits the content_rectangle exactly.
            ttlet window_aperture_clipping_rectangle =
                intersect(_window_clipping_rectangle, mat::T2{_window_rectangle} * aperture_rectangle);

            // Update layout parameters for each child.
            content->set_layout_parameters(mat::T2{_window_rectangle} * content_rectangle, window_aperture_clipping_rectangle);
            if constexpr (can_scroll_horizontally) {
                horizontal_scroll_bar->set_layout_parameters(
                    mat::T2{_window_rectangle} * horizontal_scroll_bar_rectangle, _window_clipping_rectangle);
            }
            if constexpr (can_scroll_vertically) {
                vertical_scroll_bar->set_layout_parameters(
                    mat::T2{_window_rectangle} * vertical_scroll_bar_rectangle, _window_clipping_rectangle);
            }
        }

        need_redraw |= content->updateLayout(display_time_point, need_layout);
        if constexpr (can_scroll_horizontally) {
            need_redraw |= horizontal_scroll_bar->updateLayout(display_time_point, need_layout);
        }
        if constexpr (can_scroll_vertically) {
            need_redraw |= vertical_scroll_bar->updateLayout(display_time_point, need_layout);
        }

        if (can_scroll_horizontally) {
            horizontal_scroll_bar->mutex.unlock();
        }
        if (can_scroll_vertically) {
            vertical_scroll_bar->mutex.unlock();
        }

        return Widget::updateLayout(display_time_point, need_layout) || need_redraw;
    }

    void draw(DrawContext context, hires_utc_clock::time_point display_time_point) noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());
        tt_assume(content);

        if constexpr (can_scroll_horizontally) {
            ttlet bar_lock = std::scoped_lock{horizontal_scroll_bar->mutex};
            horizontal_scroll_bar->draw(horizontal_scroll_bar->makeDrawContext(context), display_time_point);
        }
        if constexpr (can_scroll_vertically) {
            ttlet bar_lock = std::scoped_lock{vertical_scroll_bar->mutex};
            vertical_scroll_bar->draw(vertical_scroll_bar->makeDrawContext(context), display_time_point);
        }

        ttlet content_lock = std::scoped_lock(content->mutex);
        content->draw(content->makeDrawContext(context), display_time_point);

        Widget::draw(std::move(context), display_time_point);
    }

    [[nodiscard]] HitBox hitBoxTest(vec window_position) const noexcept override
    {
        ttlet lock = std::scoped_lock(mutex);
        tt_assume(content);

        auto r = HitBox{};

        if (_window_clipping_rectangle.contains(window_position) &&
            shrink(_window_rectangle, Theme::margin).contains(window_position)) {
            // Claim mouse events for scrolling.
            r = std::max(r, HitBox{this, _draw_layer});
        }

        r = std::max(r, content->hitBoxTest(window_position));
        if constexpr (can_scroll_horizontally) {
            r = std::max(r, horizontal_scroll_bar->hitBoxTest(window_position));
        }
        if constexpr (can_scroll_vertically) {
            r = std::max(r, vertical_scroll_bar->hitBoxTest(window_position));
        }
        return r;
    }

    Widget const *nextKeyboardWidget(Widget const *currentKeyboardWidget, bool reverse) const noexcept
    {
        ttlet lock = std::scoped_lock(mutex);
        tt_assume(content);

        // Scrollbars are never keyboard focus targets.
        return content->nextKeyboardWidget(currentKeyboardWidget, reverse);
    }

    template<typename WidgetType = GridLayoutWidget, typename... Args>
    WidgetType &setContent(Args const &... args) noexcept
    {
        ttlet lock = std::scoped_lock(mutex);

        auto widget_ptr = std::make_unique<WidgetType>(window, this, args...);
        auto &widget = *widget_ptr.get();
        content = std::move(widget_ptr);

        requestConstraint = true;
        return widget;
    }

    bool handleMouseEvent(MouseEvent const &event) noexcept override
    {
        ttlet lock = std::scoped_lock(mutex);

        if (Widget::handleMouseEvent(event)) {
            return true;

        } else if (event.type == MouseEvent::Type::Wheel) {
            scroll_offset_x += event.wheelDelta.x();
            scroll_offset_y += event.wheelDelta.y();
            requestLayout = true;
            return true;

        } else if (parent) {
            return parent->handleMouseEvent(event);
        }
        return false;
    }

private:
    std::unique_ptr<Widget> content;
    std::unique_ptr<ScrollBarWidget<false>> horizontal_scroll_bar;
    std::unique_ptr<ScrollBarWidget<true>> vertical_scroll_bar;

    observable<float> scroll_content_width;
    observable<float> scroll_content_height;
    observable<float> scroll_aperture_width;
    observable<float> scroll_aperture_height;
    observable<float> scroll_offset_x;
    observable<float> scroll_offset_y;
};

using VerticalScrollViewWidget = ScrollViewWidget<false, true>;
using HorizontalScrollViewWidget = ScrollViewWidget<true, false>;

} // namespace tt