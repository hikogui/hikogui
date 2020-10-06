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
            ttlet width = can_scroll_x ? finterval{Theme::width, child->preferred_size().width().minimum()} :
                                         child->preferred_size().width();

            ttlet height = can_scroll_y ? finterval{Theme::height, child->preferred_size().height().minimum()} :
                                          child->preferred_size().height();

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
            ttlet overflow_size = max(vec{}, child_minimum_size - _window_rectangle.extent());

            // Clamp the scroll-position by how much the child-widget is larger than the scroll-widget.
            scroll_position = vec::point(min(scroll_position, overflow_size));

            ttlet child_size =
                vec{can_scroll_x ? child_minimum_size.width() : _window_rectangle.width(),
                    can_scroll_y ? child_minimum_size.height() : _window_rectangle.height()};

            child->set_layout_parameters(
                mat::T2{_window_rectangle} * aarect{scroll_position, child_size},
                intersect(_window_rectangle, _window_clipping_rectangle));
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

private:
    std::unique_ptr<Widget> child;
    vec scroll_position;
};

using VerticalScrollWidget = ScrollWidget<false, true>;

} // namespace tt