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

    TabWidget(Window &window, Widget *parent) noexcept : Widget(window, parent)
    {
        margin = 0.0f;
    }

    ~TabWidget() {}

    [[nodiscard]] bool updateConstraints() noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());

        auto has_constrainted = Widget::updateConstraints();

        // Recurse into the selected widget.
        tt_assume(selected_tab_index >= 0 && selected_tab_index < std::ssize(tabs));
        auto &selected_tab = tabs[selected_tab_index];
        auto &child = selected_tab.widget;
        ttlet child_lock = std::scoped_lock(child->mutex);
        has_constrainted |= child->updateConstraints();

        if (has_constrainted) {
            auto tab_text_width = Theme::iconSize;
            auto tab_text_height = 0.0f;
            for (auto &&tab : tabs) {
                tab.image_cell = tab.image.makeCell();
                tab.text_cell = std::make_unique<TextCell>(*tab.text, theme->labelStyle);
                tab_text_width = std::max(tab_text_width, tab.text_cell->preferredExtent().width());
                tab_text_height = std::max(tab_text_height, tab.text_cell->preferredExtent().height());
            }
            tab_width = std::ceil(tab_text_width + Theme::margin * 2.0f);
            ttlet tab_height = tab_text_height + Theme::iconSize + Theme::margin * 3.0f;

            ttlet header_width = std::ssize(tabs) * tab_width + std::ssize(tabs) * Theme::margin;
            header_height = tab_height;

            _preferred_size = max(child->preferred_size() + vec{0.0f, header_height}, vec{header_width, 0.0f});
            return true;
        } else {
            return false;
        }
    }

    [[nodiscard]] WidgetUpdateResult
    updateLayout(hires_utc_clock::time_point displayTimePoint, bool forceLayout) noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());
        tt_assume(selected_tab_index >= 0 && selected_tab_index < std::ssize(tabs));

        content_rectangle = aarect{0.0f, 0.0f, rectangle().width(), rectangle().height() - header_height};
        header_rectangle = aarect{0.0f, content_rectangle.height(), rectangle().width(), header_height};

        auto has_laid_out = Widget::updateLayout(displayTimePoint, forceLayout);
        auto &selected_child = tabs[selected_tab_index];
        auto &child = selected_child.widget;

        ttlet child_lock = std::scoped_lock(child->mutex);
        ttlet child_window_rectangle = mat::T2(window_rectangle()) * content_rectangle;
        child->set_window_rectangle(child_window_rectangle);

        ttlet child_base_line =
            child->preferred_base_line().position(child_window_rectangle.bottom(), child_window_rectangle.top());
        child->set_window_base_line(child_base_line);

        has_laid_out |= (child->updateLayout(displayTimePoint, forceLayout) & WidgetUpdateResult::Children);
        if (has_laid_out >= WidgetUpdateResult::Self) {
            if (std::ssize(tabs) != 0) {
                auto x = Theme::margin;
                for (auto &tab : tabs) {
                    tab.tab_rect = aarect{header_rectangle.x() + x, header_rectangle.y(), tab_width, header_rectangle.height()};
                    auto tab_inner_rect = shrink(tab.tab_rect, Theme::margin);

                    auto icon_size = aarect{Theme::iconSize, Theme::iconSize};
                    tab.image_rect = align(tab_inner_rect, icon_size, Alignment::TopCenter);

                    auto text_size = aarect{tab_inner_rect.width(), tab_inner_rect.height() - Theme::iconSize - Theme::margin};
                    tab.text_rect = align(tab_inner_rect, text_size, Alignment::BottomCenter);

                    x += tab_width;
                    x += Theme::margin;
                }
            }
        }

        return has_laid_out;
    }

    void drawTab(DrawContext drawContext, TabEntry const &tab, bool tab_is_selected, bool hover_over_tab, bool tab_is_pressed) noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());

        if (tab_is_pressed) {
            drawContext.fillColor = theme->fillColor(nestingLevel() + 1);
        } else if (tab_is_selected || hover_over_tab) {
            drawContext.fillColor = theme->fillColor(nestingLevel());
        } else {
            drawContext.fillColor = theme->fillColor(nestingLevel() - 1);
        }
        drawContext.color = theme->fillColor(nestingLevel());
        drawContext.cornerShapes = vec{0.0f, 0.0f, Theme::roundingRadius, Theme::roundingRadius};
        ttlet extended_tab_rect = aarect{
            tab.tab_rect.x(), tab.tab_rect.y() - 1.0f, tab.tab_rect.width(), tab.tab_rect.height() + 1
        };
        drawContext.drawBoxIncludeBorder(extended_tab_rect);

        drawContext.transform = mat::T{0.0f, 0.0f, 0.001f} * drawContext.transform;
        drawContext.color = theme->foregroundColor;
        tab.image_cell->draw(drawContext, tab.image_rect, Alignment::MiddleCenter, tab.image_rect.middle(), true);
        tab.text_cell->draw(drawContext, tab.text_rect, Alignment::MiddleCenter, tab.text_rect.middle(), true);
    }

    void drawHeader(DrawContext const &drawContext) noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());

        for (ssize_t i = 0; i != std::ssize(tabs); ++i) {
            ttlet &tab = tabs[i];

            drawTab(drawContext, tab, i == selected_tab_index, i == hover_tab_index, i == pressed_tab_index);
        }
    }

    void drawChild(DrawContext context, hires_utc_clock::time_point displayTimePoint, Widget &child) noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());

        // Draw a background for the child.
        context.fillColor = theme->fillColor(nestingLevel());
        context.drawFilledQuad(content_rectangle);

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

        drawHeader(drawContext);

        if (selected_tab_index >= 0 && selected_tab_index < std::ssize(tabs)) {
            auto &tab = tabs[selected_tab_index];
            drawChild(drawContext, displayTimePoint, *tab.widget);
        }
    }

    void handleMouseEvent(MouseEvent const &event) noexcept override
    {
        if (event.type == MouseEvent::Type::ButtonUp && event.cause.leftButton) {
            if (pressed_tab_index != -1) {
                selected_tab_index = pressed_tab_index;
                requestConstraint = true;
                window.requestResize = true;
            }
        }

        ssize_t new_hover_tab_index = -1;
        for (ssize_t i = 0; i != std::ssize(tabs); ++i) {
            if (tabs[i].tab_rect.contains(event.position)) {
                new_hover_tab_index = i;
            }
        }

        auto state_has_changed = false;
        state_has_changed |= compare_then_assign(hover_tab_index, new_hover_tab_index);
        state_has_changed |= compare_then_assign(pressed_tab_index, event.down.leftButton ? new_hover_tab_index : -1);

        if (state_has_changed) {
            window.requestRedraw = true;
        }
    }

    [[nodiscard]] HitBox hitBoxTest(vec position) const noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());

        auto r =
            header_rectangle.contains(position) ? HitBox(this, elevation, HitBox::Type::Button) :
            rectangle().contains(position) ? HitBox{this, elevation} :
            HitBox{};

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
    ssize_t hover_tab_index = -1;
    ssize_t pressed_tab_index = -1;

    float tab_width;
    float header_height;
    aarect header_rectangle;
    aarect content_rectangle;
};

} // namespace tt