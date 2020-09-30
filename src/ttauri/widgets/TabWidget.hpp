// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "Widget.hpp"
#include "GridWidget.hpp"
#include "../cells/ImageCell.hpp"
#include "../cells/TextCell.hpp"

namespace tt {

class TabWidget : public Widget {
public:
    using text_type = observable<std::u8string>;

    template<typename V>
    TabWidget(Window &window, Widget *parent, V &&value) noexcept : Widget(window, parent), value(std::forward<V>(value))
    {
        [[maybe_unused]] ttlet value_cbid = value.add_callback([this](auto...) {
            this->requestConstraint = true;
        });

        margin = 0.0f;
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
            child->set_window_rectangle(window_rectangle());
            child->set_window_base_line(window_base_line());
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

    void draw(DrawContext const &drawContext, hires_utc_clock::time_point displayTimePoint) noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());
        tt_assume(*value >= 0 && *value <= std::ssize(children));

        auto &child = children[*value];
        drawChild(drawContext, displayTimePoint, *child);
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

protected:
    std::vector<std::unique_ptr<Widget>> children;
    observable<int> value = 0;
};

} // namespace tt