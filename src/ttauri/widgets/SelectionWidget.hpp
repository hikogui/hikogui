// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "Widget.hpp"
#include "WindowWidget.hpp"
#include "overlay_view_widget.hpp"
#include "ScrollViewWidget.hpp"
#include "RowColumnLayoutWidget.hpp"
#include "../stencils/text_stencil.hpp"
#include "../GUI/DrawContext.hpp"
#include "../text/FontBook.hpp"
#include "../text/ElusiveIcons.hpp"
#include "../observable.hpp"
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>

namespace tt {

template<typename ValueType>
class SelectionWidget final : public widget {
public:
    using value_type = ValueType;
    using option_list_type = std::vector<std::pair<ValueType, l10n_label>>;

    observable<l10n_label> unknown_label;
    observable<value_type> value;
    observable<option_list_type> option_list;

    template<typename Value = value_type, typename OptionList = option_list_type, typename UnknownLabel = l10n_label>
    SelectionWidget(
        Window &window,
        std::shared_ptr<widget> parent,
        Value &&value = value_type{},
        OptionList &&option_list = option_list_type{},
        UnknownLabel &&unknown_label = l10n_label{l10n(u8"<unknown>")}) noexcept :
        widget(window, parent),
        value(std::forward<Value>(value)),
        option_list(std::forward<OptionList>(option_list)),
        unknown_label(std::forward<UnknownLabel>(unknown_label))
    {
    }

    ~SelectionWidget() {}

    void initialize() noexcept override
    {
        overlay_widget = std::make_shared<overlay_view_widget>(window, shared_from_this());
        overlay_widget->initialize();

        scroll_widget = overlay_widget->make_widget<VerticalScrollViewWidget<>>();
        column_widget = scroll_widget->make_widget<ColumnLayoutWidget>();

        repopulate_options();

        _value_callback = this->value.subscribe([this](auto...) {
            _request_reconstrain = true;
        });
        _option_list_callback = this->option_list.subscribe([this](auto...) {
            repopulate_options();
            _request_reconstrain = true;
        });
        _unknown_label_callback = this->unknown_label.subscribe([this](auto...) {
            _request_reconstrain = true;
        });
    }

    [[nodiscard]] bool update_constraints() noexcept override
    {
        tt_assume(GUISystem_mutex.recurse_lock_count());

        auto updated = widget::update_constraints();
        if (selecting) {
            updated |= overlay_widget->update_constraints();
        }

        if (updated) {
            ttlet index = get_value_as_index();
            if (index == -1) {
                text_stencil = (*unknown_label).make_stencil(Alignment::MiddleLeft, theme->placeholderLabelStyle);
                text_stencil_color = theme->placeholderLabelStyle.color;
            } else {
                text_stencil = (*option_list)[index].second.make_stencil(Alignment::MiddleLeft, theme->labelStyle);
                text_stencil_color = theme->labelStyle.color;
            }

            auto text_size =
                (*unknown_label).make_stencil(Alignment::MiddleLeft, theme->placeholderLabelStyle)->preferred_extent();
            for (ttlet & [ tag, text ] : *option_list) {
                text_size = max(text_size, text.make_stencil(Alignment::MiddleLeft, theme->labelStyle)->preferred_extent());
            }

            _preferred_size =
                interval_vec2::make_minimum(text_size) + vec{Theme::smallSize + Theme::margin * 2.0f, Theme::margin * 2.0f};
            _preferred_base_line = relative_base_line{VerticalAlignment::Middle, 0.0f, 200.0f};
            return true;

        } else {
            return false;
        }
    }

    [[nodiscard]] bool update_layout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept override
    {
        tt_assume(GUISystem_mutex.recurse_lock_count());

        need_layout |= std::exchange(_request_relayout, false);

        if (selecting) {
            if (need_layout) {
                // The overlay itself will make sure the overlay fits the window, so we give the preferred size and position
                // from the point of view of the selection widget.

                // The overlay should start on the same left edge as the selection box and the same width.
                // The height of the overlay should be the maximum height, which will show all the options.

                ttlet overlay_width = clamp(rectangle().width() - Theme::smallSize, overlay_widget->preferred_size().width());
                ttlet overlay_height = overlay_widget->preferred_size().maximum().height();
                ttlet overlay_x = _window_rectangle.x() + Theme::smallSize;
                ttlet overlay_y = std::round(_window_rectangle.middle() - overlay_height * 0.5f);
                ttlet overlay_rectangle = aarect{overlay_x, overlay_y, overlay_width, overlay_height};

                overlay_widget->set_layout_parameters(overlay_rectangle, overlay_rectangle);
            }
            need_layout |= overlay_widget->update_layout(display_time_point, need_layout);
        }

        if (need_layout) {
            left_box_rectangle = aarect{0.0f, 0.0f, Theme::smallSize, rectangle().height()};
            chevrons_glyph = to_FontGlyphIDs(ElusiveIcon::ChevronUp);
            ttlet chevrons_glyph_bbox = PipelineSDF::DeviceShared::getBoundingBox(chevrons_glyph);
            chevrons_rectangle = align(left_box_rectangle, scale(chevrons_glyph_bbox, Theme::iconSize), Alignment::MiddleCenter);

            // The unknown_label is located to the right of the selection box icon.
            option_rectangle = aarect{
                left_box_rectangle.right() + Theme::margin,
                0.0f,
                rectangle().width() - left_box_rectangle.width() - Theme::margin * 2.0f,
                rectangle().height()};

            text_stencil->set_layout_parameters(option_rectangle, base_line());
        }
        return widget::update_layout(display_time_point, need_layout);
    }

    void draw(DrawContext context, hires_utc_clock::time_point display_time_point) noexcept override
    {
        tt_assume(GUISystem_mutex.recurse_lock_count());

        drawOutline(context);
        drawLeftBox(context);
        drawChevrons(context);
        drawValue(context);

        if (selecting) {
            overlay_widget->draw(overlay_widget->make_draw_context(context), display_time_point);
        }

        widget::draw(std::move(context), display_time_point);
    }

    bool handle_mouse_event(MouseEvent const &event) noexcept override
    {
        ttlet lock = std::scoped_lock(GUISystem_mutex);
        auto handled = widget::handle_mouse_event(event);

        if (event.cause.leftButton) {
            handled = true;
            if (*enabled) {
                if (event.type == MouseEvent::Type::ButtonUp && _window_rectangle.contains(event.position)) {
                    handle_command(command::gui_activate);
                }
            }
        }
        return handled;
    }

    bool handle_command(command command) noexcept override
    {
        ttlet lock = std::scoped_lock(GUISystem_mutex);
        auto handled = widget::handle_command(command);

        if (*enabled) {
            if (command == command::gui_activate) {
                handled = true;
                selecting = !selecting;
            }
        }

        _request_relayout = true;
        return handled;
    }

    [[nodiscard]] HitBox hitbox_test(vec window_position) const noexcept override
    {
        ttlet lock = std::scoped_lock(GUISystem_mutex);
        ttlet position = _from_window_transform * window_position;

        auto r = HitBox{};

        if (selecting) {
            r = std::max(r, overlay_widget->hitbox_test(window_position));
        }

        if (_window_clipping_rectangle.contains(window_position)) {
            r = std::max(r, HitBox{weak_from_this(), _draw_layer, *enabled ? HitBox::Type::Button : HitBox::Type::Default});
        }

        return r;
    }

    [[nodiscard]] bool accepts_focus() const noexcept override
    {
        tt_assume(GUISystem_mutex.recurse_lock_count());
        return *enabled;
    }

    /** Populate the scroll view with menu items corresponding to the options.
     */
    void repopulate_options() noexcept
    {
        ttlet lock = std::scoped_lock(GUISystem_mutex);

        column_widget->clear();
        ttlet option_list_ = *option_list;
        for (ttlet & [ tag, text ] : std::views::reverse(option_list_)) {
            auto button = column_widget->make_widget<button_widget>();
            button->label = text;
        }
    }

private:
    typename decltype(unknown_label)::callback_ptr_type _unknown_label_callback;
    typename decltype(value)::callback_ptr_type _value_callback;
    typename decltype(option_list)::callback_ptr_type _option_list_callback;

    std::unique_ptr<stencil> text_stencil;
    vec text_stencil_color;

    aarect option_rectangle;
    aarect left_box_rectangle;

    FontGlyphIDs chevrons_glyph;
    aarect chevrons_rectangle;

    bool selecting = false;
    std::shared_ptr<overlay_view_widget> overlay_widget;
    std::shared_ptr<VerticalScrollViewWidget<>> scroll_widget;
    std::shared_ptr<ColumnLayoutWidget> column_widget;

    [[nodiscard]] ssize_t get_value_as_index() const noexcept
    {
        ssize_t index = 0;
        for (ttlet & [ tag, unknown_label_text ] : *option_list) {
            if (value == tag) {
                return index;
            }
            ++index;
        }

        return -1;
    }

    void drawOutline(DrawContext drawContext) noexcept
    {
        tt_assume(GUISystem_mutex.recurse_lock_count());

        drawContext.cornerShapes = Theme::roundingRadius;
        drawContext.drawBoxIncludeBorder(rectangle());
    }

    void drawLeftBox(DrawContext drawContext) noexcept
    {
        tt_assume(GUISystem_mutex.recurse_lock_count());

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
        tt_assume(GUISystem_mutex.recurse_lock_count());

        drawContext.transform = mat::T{0.0, 0.0, 0.2f} * drawContext.transform;
        drawContext.color = *enabled ? theme->foregroundColor : drawContext.fillColor;
        drawContext.drawGlyph(chevrons_glyph, chevrons_rectangle);
    }

    void drawValue(DrawContext drawContext) noexcept
    {
        tt_assume(GUISystem_mutex.recurse_lock_count());

        drawContext.transform = mat::T{0.0, 0.0, 0.1f} * drawContext.transform;
        drawContext.color = *enabled ? text_stencil_color : drawContext.color;
        text_stencil->draw(drawContext, true);
    }
};

} // namespace tt
