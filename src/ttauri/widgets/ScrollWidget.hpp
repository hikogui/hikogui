// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "Widget.hpp"
#include "GridWidget.hpp"
#include "../cells/ImageCell.hpp"
#include "../cells/TextCell.hpp"

namespace tt {

template<bool CanScrollX = false, bool CanScrollY = true>
class ScrollWidget final : public Widget {
public:
    static constexpr bool can_scroll_x = CanScrollX;
    static constexpr bool can_scroll_y = CanScrollY;

    ScrollWidget(Window &window, Widget *parent) noexcept : Widget(window, parent)
    {
        if (parent) {
            // The tab-widget will not draw itself, only its selected child.
            ttlet lock = std::scoped_lock(parent->mutex);
            _draw_layer = parent->draw_layer();
            _semantic_layer = parent->semantic_layer();
        }
        _margin = 0.0f;
    }

    ~ScrollWidget() {}

    [[nodiscard]] bool updateConstraints() noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());
        tt_assume(child);

        auto has_updated_contraints = Widget::updateConstraints();
        if (has_updated_contraints) {
            // The value has changed, so resize the window.
            window.requestResize = true;
        }

        // Recurse into the selected widget.
        ttlet child_lock = std::scoped_lock(child->mutex);
        if (child->updateConstraints() || has_updated_contraints) {
            auto width = child->preferred_size().width();
            auto height = child->preferred_size().height();

            if (can_scroll_x) {
                width = {Theme::width, width.minimum()};
            }

            if (can_scroll_y) {
                height = {Theme::height, height.minimum()};
            }

            // Make room for the scroll bars.
            if (can_scroll_x) {
                height += Theme::smallSize;
            }

            if (can_scroll_y) {
                width += Theme::smallSize;
            }

            _preferred_size = interval_vec2{width, height};
            _preferred_base_line = {};
            return true;
        } else {
            return false;
        }
    }

    [[nodiscard]] bool updateLayout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());
        tt_assume(child);

        ttlet child_lock = std::scoped_lock(child->mutex);
        auto need_redraw = need_layout |= std::exchange(requestLayout, false);
        if (need_layout) {
            ttlet child_minimum_size = child->preferred_size().minimum();
            overflow_size = max(vec{}, child_minimum_size - _window_rectangle.extent());

            scroll_offset = clamp(scroll_offset, vec{}, overflow_size);

            auto width = _window_rectangle.width();
            auto height = _window_rectangle.height();
            if (can_scroll_x) {
                height -= Theme::smallSize;
            }
            if (can_scroll_y) {
                width -= Theme::smallSize;
            }
            if (can_scroll_x) {
                width = child_minimum_size.width();
            }
            if (can_scroll_y) {
                height = child_minimum_size.height();
            }

            ttlet child_position = vec::point(-scroll_offset);
            ttlet child_size = vec{width, height};

            content_rectangle = aarect{
                rectangle().x(),
                rectangle().y() + (can_scroll_x ? Theme::smallSize : 0.0f),
                rectangle().width() - (can_scroll_y ? Theme::smallSize : 0.0f),
                rectangle().height() - (can_scroll_x ? Theme::smallSize : 0.0f)
            };

            vertical_scroll_bar_rectangle = aarect{
                rectangle().right() - Theme::smallSize,
                rectangle().y() + Theme::smallSize,
                Theme::smallSize,
                rectangle().height() - Theme::smallSize};

            horizontal_scroll_bar_rectangle = aarect{
                rectangle().x(),
                rectangle().y(),
                rectangle().width() - Theme::smallSize,
                Theme::smallSize
            };

            ttlet window_content_clipping_rectangle = mat::T2(_window_rectangle) * content_rectangle;

            child->set_layout_parameters(
                mat::T2{_window_rectangle} * aarect{child_position, child_size},
                intersect(_window_rectangle, window_content_clipping_rectangle));
        }

        need_redraw |= child->updateLayout(display_time_point, need_layout);
        return Widget::updateLayout(display_time_point, need_layout) || need_redraw;
    }

    void draw(DrawContext context, hires_utc_clock::time_point display_time_point) noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());
        tt_assume(child);

        ttlet child_lock = std::scoped_lock(child->mutex);
        child->draw(child->makeDrawContext(context), display_time_point);

        if (can_scroll_y) {
            drawVerticalScrollBar(context);
        }

        Widget::draw(std::move(context), display_time_point);
    }

    [[nodiscard]] HitBox hitBoxTest(vec window_position) const noexcept override
    {
        ttlet lock = std::scoped_lock(mutex);
        tt_assume(child);

        return child->hitBoxTest(window_position);
    }

    Widget const *nextKeyboardWidget(Widget const *currentKeyboardWidget, bool reverse) const noexcept
    {
        ttlet lock = std::scoped_lock(mutex);
        tt_assume(child);

        return child->nextKeyboardWidget(currentKeyboardWidget, reverse);
    }

    template<typename WidgetType = GridWidget, typename... Args>
    WidgetType &setContent(Args const &... args) noexcept
    {
        ttlet lock = std::scoped_lock(mutex);

        auto widget_ptr = std::make_unique<WidgetType>(window, this, args...);
        auto &widget = *widget_ptr.get();
        child = std::move(widget_ptr);

        requestConstraint = true;
        return widget;
    }

    bool handleMouseEvent(MouseEvent const &event) noexcept override
    {
        ttlet lock = std::scoped_lock(mutex);

        if (Widget::handleMouseEvent(event)) {
            return true;

        } else if (event.type == MouseEvent::Type::Wheel) {
            scroll_offset += event.wheelDelta;
            requestLayout = true;
            return true;

        } else if (parent) {
            return parent->handleMouseEvent(event);
        }
        return false;
    }

private:
    std::unique_ptr<Widget> child;
    vec overflow_size = vec{};
    vec scroll_offset = vec{};

    aarect content_rectangle;
    aarect horizontal_scroll_bar_rectangle;
    aarect vertical_scroll_bar_rectangle;

    void drawVerticalScrollBar(DrawContext context) noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());

        context.fillColor = theme->fillColor(_semantic_layer + 1);
        context.drawFilledQuad(vertical_scroll_bar_rectangle);
    }
};

using VerticalScrollWidget = ScrollWidget<false, true>;

} // namespace tt