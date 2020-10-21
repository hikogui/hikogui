// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "Widget.hpp"
#include "WindowWidget.hpp"
#include "overlay_view_widget.hpp"
#include "ScrollViewWidget.hpp"
#include "RowColumnLayoutWidget.hpp"
#include "../cells/TextCell.hpp"
#include "../GUI/DrawContext.hpp"
#include "../text/FontBook.hpp"
#include "../text/format10.hpp"
#include "../text/ElusiveIcons.hpp"
#include "../observable.hpp"
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>

namespace tt {

template<typename ValueType>
class SelectionWidget final : public Widget {
public:
    using option_list_type = observable<std::vector<std::pair<ValueType, std::u8string>>>;
    using value_type = observable<ValueType>;
    using label_type = observable<std::u8string>;

    label_type label;
    value_type value;
    option_list_type option_list;

    SelectionWidget(
        Window &window,
        Widget *parent,
        value_type const &value = {},
        option_list_type const &option_list = {},
        label_type const &label = l10n(u8"<unknown>")) noexcept :
        Widget(window, parent),
        value(value),
        option_list(option_list),
        label(label),
        overlay_widget(std::make_unique<overlay_view_widget>(window, this)),
        scroll_widget(overlay_widget->makeWidget<VerticalScrollViewWidget<>>()),
        column_widget(scroll_widget.makeWidget<ColumnLayoutWidget>())
    {
        repopulate_options();

        _value_callback = scoped_callback(this->value, [this](auto...) {
            request_reconstrain = true;
        });
        _option_list_callback = scoped_callback(this->option_list, [this](auto...) {
            repopulate_options();
            request_reconstrain = true;
        });
        _label_callback = scoped_callback(this->label, [this](auto...) {
            request_reconstrain = true;
        });
    }

    template<typename... Args>
    SelectionWidget(
        Window &window,
        Widget *parent,
        value_type const &value,
        option_list_type const &option_list,
        l10n const &fmt,
        Args const &... args) noexcept :
        SelectionWidget(window, parent, value, option_list, format(fmt, args...))
    {
    }

    template<typename... Args>
    SelectionWidget(
        Window &window,
        Widget *parent,
        option_list_type const &option_list,
        l10n const &fmt,
        Args const &... args) noexcept :
        SelectionWidget(window, parent, value_type{}, option_list, format(fmt, args...))
    {
    }

    template<typename... Args>
    SelectionWidget(Window &window, Widget *parent, value_type const &value, l10n const &fmt, Args const &... args) noexcept :
        SelectionWidget(window, parent, value, option_list_type{}, format(fmt, args...))
    {
    }

    template<typename... Args>
    SelectionWidget(Window &window, Widget *parent, l10n const &fmt, Args const &... args) noexcept :
        SelectionWidget(window, parent, value_type{}, option_list_type{}, format(fmt, args...))
    {
    }

    ~SelectionWidget() {}

    [[nodiscard]] bool update_constraints() noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());

        auto updated = Widget::update_constraints();
        if (selecting) {
            ttlet child_lock = std::scoped_lock(overlay_widget->mutex);
            updated |= overlay_widget->update_constraints();
        }

        if (updated) {
            ttlet index = get_value_as_index();
            if (index == -1) {
                text_cell = std::make_unique<TextCell>(*label, theme->placeholderLabelStyle);
            } else {
                text_cell = std::make_unique<TextCell>((*option_list)[index].second, theme->labelStyle);
            }

            auto text_size = TextCell(*label, theme->placeholderLabelStyle).preferredExtent();
            for (ttlet & [ tag, text ] : *option_list) {
                text_size = max(text_size, TextCell(text, theme->labelStyle).preferredExtent());
            }

            p_preferred_size =
                interval_vec2::make_minimum(text_size) + vec{Theme::smallSize + Theme::margin * 2.0f, Theme::margin * 2.0f};
            p_preferred_base_line = relative_base_line{VerticalAlignment::Middle, 0.0f, 200.0f};
            return true;

        } else {
            return false;
        }
    }

    [[nodiscard]] bool update_layout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());

        need_layout |= std::exchange(request_relayout, false);

        if (selecting) {
            ttlet child_lock = std::scoped_lock(overlay_widget->mutex);

            if (need_layout) {
                // The overlay itself will make sure the overlay fits the window, so we give the preferred size and position
                // from the point of view of the selection widget.

                // The overlay should start on the same left edge as the selection box and the same width.
                // The height of the overlay should be the maximum height, which will show all the options.

                ttlet overlay_width = clamp(rectangle().width() - Theme::smallSize, overlay_widget->preferred_size().width());
                ttlet overlay_height = overlay_widget->preferred_size().maximum().height();
                ttlet overlay_x = p_window_rectangle.x() + Theme::smallSize;
                ttlet overlay_y = std::round(p_window_rectangle.middle() - overlay_height * 0.5f);
                ttlet overlay_rectangle = aarect{overlay_x, overlay_y, overlay_width, overlay_height};

                overlay_widget->set_layout_parameters(overlay_rectangle, overlay_rectangle);
            }
            overlay_widget->update_layout(display_time_point, need_layout);
        }

        if (need_layout) {
            left_box_rectangle = aarect{0.0f, 0.0f, Theme::smallSize, rectangle().height()};
            chevrons_glyph = to_FontGlyphIDs(ElusiveIcon::ChevronUp);
            ttlet chevrons_glyph_bbox = PipelineSDF::DeviceShared::getBoundingBox(chevrons_glyph);
            chevrons_rectangle = align(left_box_rectangle, scale(chevrons_glyph_bbox, Theme::iconSize), Alignment::MiddleCenter);

            // The label is located to the right of the selection box icon.
            option_rectangle = aarect{
                left_box_rectangle.right() + Theme::margin,
                0.0f,
                rectangle().width() - left_box_rectangle.width() - Theme::margin * 2.0f,
                rectangle().height()};
        }
        return Widget::update_layout(display_time_point, need_layout);
    }

    void draw(DrawContext context, hires_utc_clock::time_point display_time_point) noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());

        drawOutline(context);
        drawLeftBox(context);
        drawChevrons(context);
        drawValue(context);

        if (selecting) {
            ttlet child_lock = std::scoped_lock(overlay_widget->mutex);
            overlay_widget->draw(overlay_widget->make_draw_context(context), display_time_point);
        }

        Widget::draw(std::move(context), display_time_point);
    }

    bool handle_mouse_event(MouseEvent const &event) noexcept override
    {
        ttlet lock = std::scoped_lock(mutex);
        auto handled = Widget::handle_mouse_event(event);

        if (event.cause.leftButton) {
            handled = true;
            if (*enabled) {
                if (event.type == MouseEvent::Type::ButtonUp && p_window_rectangle.contains(event.position)) {
                    handle_command(command::gui_activate);
                }
            }
        }
        return handled;
    }

    bool handle_command(command command) noexcept override
    {
        ttlet lock = std::scoped_lock(mutex);
        auto handled = Widget::handle_command(command);

        if (*enabled) {
            if (command == command::gui_activate) {
                handled = true;
                selecting = !selecting;
            }
        }

        request_relayout = true;
        return handled;
    }

    [[nodiscard]] HitBox hitbox_test(vec window_position) const noexcept override
    {
        ttlet lock = std::scoped_lock(mutex);
        ttlet position = from_window_transform * window_position;

        auto r = HitBox{};

        if (selecting) {
            r = std::max(r, overlay_widget->hitbox_test(window_position));
        }

        if (p_window_clipping_rectangle.contains(window_position)) {
            r = std::max(r, HitBox{this, p_draw_layer, *enabled ? HitBox::Type::Button : HitBox::Type::Default});
        }

        return r;
    }

    [[nodiscard]] bool accepts_focus() const noexcept override
    {
        tt_assume(mutex.is_locked_by_current_thread());
        return *enabled;
    }

    /** Populate the scroll view with menu items corresponding to the options.
     */
    void repopulate_options() noexcept
    {
        ttlet lock = std::scoped_lock(mutex);

        column_widget.clear();
        ttlet option_list_ = *option_list;
        for (ttlet & [ tag, text ] : std::views::reverse(option_list_)) {
            auto &button = column_widget.makeWidget<ButtonWidget>();
            button.label = text;
        }
    }

private:
    scoped_callback<decltype(label)> _label_callback;
    scoped_callback<decltype(value)> _value_callback;
    scoped_callback<decltype(option_list)> _option_list_callback;
    
    std::unique_ptr<TextCell> text_cell;

    aarect option_rectangle;
    aarect left_box_rectangle;

    FontGlyphIDs chevrons_glyph;
    aarect chevrons_rectangle;

    bool selecting = false;
    std::unique_ptr<overlay_view_widget> overlay_widget;
    VerticalScrollViewWidget<> &scroll_widget;
    ColumnLayoutWidget &column_widget;

    [[nodiscard]] ssize_t get_value_as_index() const noexcept
    {
        ssize_t index = 0;
        for (ttlet & [ tag, label_text ] : *option_list) {
            if (value == tag) {
                return index;
            }
            ++index;
        }

        return -1;
    }

    void drawOutline(DrawContext drawContext) noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());

        drawContext.cornerShapes = Theme::roundingRadius;
        drawContext.drawBoxIncludeBorder(rectangle());
    }

    void drawLeftBox(DrawContext drawContext) noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());

        drawContext.transform = mat::T{0.0, 0.0, 0.1f} * drawContext.transform;
        // if (*enabled && window.active) {
        //    drawContext.color = theme->accentColor;
        //}
        drawContext.fillColor = drawContext.color;
        drawContext.cornerShapes = vec{Theme::roundingRadius, 0.0f, Theme::roundingRadius, 0.0f};
        drawContext.drawBoxIncludeBorder(left_box_rectangle);
    }

    void drawChevrons(DrawContext drawContext) noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());

        drawContext.transform = mat::T{0.0, 0.0, 0.2f} * drawContext.transform;
        drawContext.color = *enabled ? theme->foregroundColor : drawContext.fillColor;
        drawContext.drawGlyph(chevrons_glyph, chevrons_rectangle);
    }

    void drawValue(DrawContext drawContext) noexcept
    {
        tt_assume(mutex.is_locked_by_current_thread());

        drawContext.transform = mat::T{0.0, 0.0, 0.1f} * drawContext.transform;
        drawContext.color = *enabled ? text_cell->style.color : drawContext.color;
        text_cell->draw(drawContext, option_rectangle, Alignment::MiddleLeft, base_line(), true);
    }
};

} // namespace tt
