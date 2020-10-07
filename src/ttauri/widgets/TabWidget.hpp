// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "Widget.hpp"
#include "GridWidget.hpp"
#include "../cells/ImageCell.hpp"
#include "../cells/TextCell.hpp"

namespace tt {

class TabWidget final : public Widget {
public:
    observable<int> value = 0;

    template<typename V>
    TabWidget(Window &window, Widget *parent, V &&value) noexcept : Widget(window, parent), value(std::forward<V>(value))
    {
        if (parent) {
            // The tab-widget will not draw itself, only its selected child.
            ttlet lock = std::scoped_lock(parent->mutex);
            _draw_layer = parent->draw_layer();
            _semantic_layer = parent->semantic_layer();
        }
        _margin = 0.0f;

        [[maybe_unused]] ttlet value_cbid = value.add_callback([this](auto...) {
            this->requestConstraint = true;
        });
    }

    ~TabWidget() {}

    [[nodiscard]] bool updateConstraints() noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());

        auto has_updated_contraints = Widget::updateConstraints();
        if (has_updated_contraints) {
            // The value has changed, so resize the window.
            window.requestResize = true;
        }

        // Recurse into the selected widget.
        tt_assume(*value >= 0 && *value < std::ssize(children));
        auto &child = children[*value];
        ttlet child_lock = std::scoped_lock(child->mutex);
        
        if (child->updateConstraints() || has_updated_contraints) {
            _preferred_size = child->preferred_size();
            _preferred_base_line = child->preferred_base_line();
            return true;
        } else {
            return false;
        }
    }

    [[nodiscard]] bool updateLayout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());
        tt_assume(*value >= 0 && *value < std::ssize(children));

        auto &child = children[*value];
        ttlet child_lock = std::scoped_lock(child->mutex);

        auto need_redraw = need_layout |= std::exchange(requestLayout, false);
        if (need_layout) {
            child->set_layout_parameters(_window_rectangle, _window_clipping_rectangle, _window_base_line);
        }

        need_redraw |= child->updateLayout(display_time_point, need_layout);
        return Widget::updateLayout(display_time_point, need_layout) || need_redraw;
    }

    void drawChild(DrawContext context, hires_utc_clock::time_point displayTimePoint, Widget &child) noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());

        ttlet child_lock = std::scoped_lock(child.mutex);
        child.draw(child.makeDrawContext(context), displayTimePoint);
    }

    void draw(DrawContext context, hires_utc_clock::time_point display_time_point) noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());
        tt_assume(*value >= 0 && *value <= std::ssize(children));

        auto &child = children[*value];
        drawChild(context, display_time_point, *child);
        Widget::draw(std::move(context), display_time_point);
    }

    bool handleMouseEvent(MouseEvent const &event) noexcept override
    {
        if (Widget::handleMouseEvent(event)) {
            return true;
        } else if (parent) {
            return parent->handleMouseEvent(event);
        }
        return false;
    }

    [[nodiscard]] HitBox hitBoxTest(vec window_position) const noexcept override
    {
        ttlet lock = std::scoped_lock(mutex);

        tt_assume(*value >= 0 && *value <= std::ssize(children));
        return children[*value]->hitBoxTest(window_position);
    }

    Widget const *nextKeyboardWidget(Widget const *currentKeyboardWidget, bool reverse) const noexcept
    {
        ttlet lock = std::scoped_lock(mutex);

        tt_assume(*value >= 0 && *value < std::ssize(children));
        return children[*value]->nextKeyboardWidget(currentKeyboardWidget, reverse);
    }

    template<typename WidgetType = GridWidget, typename... Args>
    WidgetType &addTab(Args const &... args) noexcept
    {
        ttlet lock = std::scoped_lock(mutex);

        auto widget_ptr = std::make_unique<WidgetType>(window, this, args...);
        auto &widget = *widget_ptr.get();
        children.push_back(std::move(widget_ptr));

        // Make sure a tab is selected.
        if (*value < 0 || *value >= std::ssize(children)) {
            value = 0;
        }
        requestConstraint = true;
        return widget;
    }

private:
    std::vector<std::unique_ptr<Widget>> children;
};

} // namespace tt