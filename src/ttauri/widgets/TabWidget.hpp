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

    [[nodiscard]] WidgetUpdateResult
    updateLayout(hires_utc_clock::time_point displayTimePoint, bool forceLayout) noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());
        tt_assume(*value >= 0 && *value < std::ssize(children));

        auto has_laid_out = Widget::updateLayout(displayTimePoint, forceLayout);
        auto &child = children[*value];

        ttlet child_lock = std::scoped_lock(child->mutex);
        child->set_window_rectangle(window_rectangle());
        child->set_window_base_line(window_base_line());

        has_laid_out |= (child->updateLayout(displayTimePoint, forceLayout) & WidgetUpdateResult::Children);
        return has_laid_out;
    }

    void drawChild(DrawContext context, hires_utc_clock::time_point displayTimePoint, Widget &child) noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());

        ttlet child_lock = std::scoped_lock(child.mutex);
        context.clippingRectangle = child.clipping_rectangle();
        context.transform = child.toWindowTransform;

        // The default fill and border colors.
        ttlet childNestingLevel = child.nestingLevel();
        context.color = theme->borderColor(childNestingLevel);
        context.fillColor = theme->fillColor(childNestingLevel);

        if (*child.enabled) {
            if (child.focus && window.active) {
                context.color = theme->accentColor;
            } else if (child.hover) {
                context.color = theme->borderColor(childNestingLevel + 1);
            }

            if (child.hover) {
                context.fillColor = theme->fillColor(childNestingLevel + 1);
            }

        } else {
            // Disabled, only the outline is shown.
            context.color = theme->borderColor(childNestingLevel - 1);
            context.fillColor = theme->fillColor(childNestingLevel - 1);
        }

        child.draw(context, displayTimePoint);
    }

    void draw(DrawContext const &drawContext, hires_utc_clock::time_point displayTimePoint) noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());
        tt_assume(*value >= 0 && *value <= std::ssize(children));

        auto &child = children[*value];
        drawChild(drawContext, displayTimePoint, *child);
    }

    [[nodiscard]] HitBox hitBoxTest(vec position) const noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());
        tt_assume(*value >= 0 && *value <= std::ssize(children));

        auto &child = children[*value];
        ttlet child_lock = std::scoped_lock(child->mutex);
        return child->hitBoxTest(position - child->offsetFromParent);
    }

    Widget *nextKeyboardWidget(Widget const *currentKeyboardWidget, bool reverse) const noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());
        tt_assume(*value >= 0 && *value < std::ssize(children));

        ttlet &child = children[*value];
        ttlet child_lock = std::scoped_lock(child->mutex);
        return child->nextKeyboardWidget(currentKeyboardWidget, reverse);
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