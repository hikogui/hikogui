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

    struct TabEntry {
        Image image;
        observable<std::u8string> text;
        std::unique_ptr<Widget> widget;

        std::unique_ptr<ImageCell> image_cell;
        std::unique_ptr<TextCell> text_cell;

        aarect image_rect;
        aarect text_rect;
        aarect tab_rect;

        TabEntry(Image const &image, text_type const &text, std::unique_ptr<Widget> widget) noexcept :
            image(image), text(text), widget(std::move(widget))
        {
        }
    };

    TabWidget(Window &window, Widget *parent) noexcept : Widget(window, parent) {}

    ~TabWidget() {}

    [[nodiscard]] WidgetUpdateResult updateConstraints() noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());

        auto has_constrainted = Widget::updateConstraints();
        // Recurse into the selected widget.
        if (selected_tab_index >= 0 && selected_tab_index < std::ssize(tabs)) {
            auto &tab = tabs[selected_tab_index];
            auto &child = tab.widget;

            ttlet child_lock = std::scoped_lock(child->mutex);
            has_constrainted |= (child->updateConstraints() & WidgetUpdateResult::Children);
        }

        if (has_constrainted >= WidgetUpdateResult::Self) {
            auto tab_text_width = Theme::iconSize;
            auto tab_text_height = 0.0f;
            for (auto &&tab : tabs) {
                tab.image_cell = tab.image.makeCell();
                tab.text_cell = std::make_unique<TextCell>(*tab.text, theme->labelStyle);
                tab_text_width = std::max(tab_text_width, tab.text_cell->preferredExtent().width());
                tab_text_height = std::max(tab_text_height, tab.text_cell->preferredExtent().height());
            }
            auto tab_width = tab_text_width + Theme::margin * 2.0f;
            tab_height = tab_text_height + Theme::iconSize + Theme::margin * 3.0f;

            window.replaceConstraint(minimumWidthConstraint, width >= tab_width * std::ssize(tabs));
            window.replaceConstraint(minimumHeightConstraint, height >= tab_height);

            if (selected_tab_index >= 0 && selected_tab_index < std::ssize(tabs)) {
                auto &tab = tabs[selected_tab_index];
                auto &child = tab.widget;

                window.replaceConstraint(left_constraint, left == child->left);
                window.replaceConstraint(right_constraint, right == child->right);
                window.replaceConstraint(bottom_constraint, bottom == child->bottom);
                window.replaceConstraint(top_constraint, top - tab_height == child->top);
            }
        }

        return has_constrainted;
    }

    [[nodiscard]] WidgetUpdateResult
    updateLayout(hires_utc_clock::time_point displayTimePoint, bool forceLayout) noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());
        auto has_laid_out = Widget::updateLayout(displayTimePoint, forceLayout);
        if (selected_tab_index >= 0 && selected_tab_index < std::ssize(tabs)) {
            auto &tab = tabs[selected_tab_index];
            auto &child = tab.widget;

            ttlet child_lock = std::scoped_lock(child->mutex);
            has_laid_out |= (child->updateLayout(displayTimePoint, forceLayout) & WidgetUpdateResult::Children);
        }

        if (has_laid_out >= WidgetUpdateResult::Self) {
            if (std::ssize(tabs) != 0) {
                // Spread the tabs over the width of container.
                auto tab_width = std::floor(rectangle().width() / std::ssize(tabs));

                auto x = 0.0f;
                for (auto &tab : tabs) {
                    tab.tab_rect = aarect{x, rectangle().height() - tab_height, tab_width, tab_height};
                    auto tab_inner_rect = shrink(tab.tab_rect, Theme::margin);

                    auto icon_size = aarect{Theme::iconSize, Theme::iconSize};
                    tab.image_rect = align(tab_inner_rect, icon_size, Alignment::TopCenter);

                    auto text_size = aarect{tab_inner_rect.width(), tab_inner_rect.height() - Theme::iconSize - Theme::margin};
                    tab.text_rect = align(tab_inner_rect, text_size, Alignment::BottomCenter);

                    x += tab_width;
                }
            }
        }

        return has_laid_out;
    }

    void drawChild(DrawContext context, hires_utc_clock::time_point displayTimePoint, Widget &child) noexcept
    {
        ttlet child_lock = std::scoped_lock(child.mutex);

        context.clippingRectangle = child.clippingRectangle();
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

        drawContext.drawFilledQuad(rectangle());

        if (selected_tab_index >= 0 && selected_tab_index < std::ssize(tabs)) {
            auto &tab = tabs[selected_tab_index];
            drawChild(drawContext, displayTimePoint, *tab.widget);
        }
    }

    [[nodiscard]] HitBox hitBoxTest(vec position) const noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());

        auto r = rectangle().contains(position) ? HitBox{this, elevation} : HitBox{};

        if (selected_tab_index >= 0 && selected_tab_index < std::ssize(tabs)) {
            auto &tab = tabs[selected_tab_index];
            auto &child = tab.widget;
            ttlet child_lock = std::scoped_lock(child->mutex);
            r = std::max(r, child->hitBoxTest(position - child->offsetFromParent));
        }
        return r;
    }

    [[nodiscard]] Widget *nextKeyboardWidget(Widget const *currentKeyboardWidget, bool reverse) const noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());

        if (currentKeyboardWidget == nullptr && acceptsFocus()) {
            // The first widget that accepts focus.
            return const_cast<TabWidget *>(this);

        } else {
            bool found = false;

            if (selected_tab_index >= 0 && selected_tab_index < std::ssize(tabs)) {
                auto &tab = tabs[selected_tab_index];
                auto *child = tab.widget.get();
                ttlet child_lock = std::scoped_lock(child->mutex);

                if (found) {
                    // Find the first focus accepting widget.
                    if (auto *tmp = child->nextKeyboardWidget(nullptr, reverse)) {
                        return tmp;
                    }

                } else if (child == currentKeyboardWidget) {
                    found = true;

                } else {
                    auto *tmp = child->nextKeyboardWidget(currentKeyboardWidget, reverse);
                    if (tmp == foundWidgetPtr) {
                        // The current widget was found, but no next widget available in the child.
                        found = true;

                    } else if (tmp) {
                        return tmp;
                    }
                }
            }
            return found ? foundWidgetPtr : nullptr;
        }
    }

    template<typename WidgetType = GridWidget, typename... Args>
    WidgetType &addTab(Image const &image, text_type const &text, Args const &... args) noexcept
    {
        ttlet lock = std::scoped_lock(mutex);

        auto widget_ptr = std::make_unique<WidgetType>(window, this, args...);
        auto &widget = *widget_ptr.get();
        tabs.emplace_back(image, text, std::move(widget_ptr));

        // The text of the label can be translated, so add a notifier.
        ttlet cbid = tabs.back().text.add_callback([this](auto...) {
            requestConstraint = true;
        });

        // Make sure a tab is selected.
        if (selected_tab_index >= std::ssize(tabs)) {
            selected_tab_index = 0;
        }
        requestConstraint = true;
        return widget;
    }

protected:
    std::vector<TabEntry> tabs;
    ssize_t selected_tab_index = 0;
    float tab_height = 0.0f;

    rhea::constraint left_constraint;
    rhea::constraint right_constraint;
    rhea::constraint top_constraint;
    rhea::constraint bottom_constraint;
};

} // namespace tt