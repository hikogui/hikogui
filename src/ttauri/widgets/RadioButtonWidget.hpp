// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "Widget.hpp"
#include "../GUI/DrawContext.hpp"
#include "../text/format10.hpp"
#include "../observable.hpp"
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>

namespace tt {

template<typename ValueType, ValueType ActiveValue>
class RadioButtonWidget final : public Widget {
public:
    observable<ValueType> value;
    observable<std::u8string> label;

    template<typename V, typename... Args>
    RadioButtonWidget(Window &window, std::shared_ptr<Widget> parent, V &&value, l10n const &fmt, Args const &... args) noexcept :
        Widget(window, parent), value(std::forward<V>(value)), label(format(fmt, args...))
    {
    }

    template<typename V>
    RadioButtonWidget(Window &window, std::shared_ptr<Widget> parent, V &&value) noexcept :
        RadioButtonWidget(window, parent, std::forward<V>(value), l10n{})
    {
    }

    RadioButtonWidget(Window &window, std::shared_ptr<Widget> parent) noexcept :
        RadioButtonWidget(window, parent, observable<ValueType>{}, l10n{})
    {
    }

    ~RadioButtonWidget() {}

    void initialize() noexcept override
    {
        value_callback = value.subscribe([this](auto...) {
            this->window.requestRedraw = true;
        });
        label_callback = label.subscribe([this](auto...) {
            request_reconstrain = true;
        });
    }

    [[nodiscard]] bool update_constraints() noexcept override
    {
        tt_assume(GUISystem_mutex.recurse_lock_count());

        if (Widget::update_constraints()) {
            labelCell = std::make_unique<TextCell>(*label, theme->labelStyle);

            ttlet minimumHeight = std::max(labelCell->preferredExtent().height(), Theme::smallSize);
            ttlet minimumWidth = Theme::smallSize + Theme::margin + labelCell->preferredExtent().width();

            p_preferred_size = interval_vec2::make_minimum(minimumWidth, minimumHeight);
            p_preferred_base_line = relative_base_line{VerticalAlignment::Top, -Theme::smallSize * 0.5f};
            return true;
        } else {
            return false;
        }
    }

    [[nodiscard]] bool update_layout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept override
    {
        tt_assume(GUISystem_mutex.recurse_lock_count());

        need_layout |= std::exchange(request_relayout, false);
        if (need_layout) {
            radioButtonRectangle = aarect{0.0f, base_line() - Theme::smallSize * 0.5f, Theme::smallSize, Theme::smallSize};

            ttlet labelX = radioButtonRectangle.p3().x() + Theme::margin;
            _label_rectangle = aarect{labelX, 0.0f, rectangle().width() - labelX, rectangle().height()};

            pipRectangle = shrink(radioButtonRectangle, 1.5f);
        }
        return Widget::update_layout(display_time_point, need_layout);
    }

    void draw(DrawContext context, hires_utc_clock::time_point display_time_point) noexcept override
    {
        tt_assume(GUISystem_mutex.recurse_lock_count());

        drawRadioButton(context);
        drawPip(context);
        draw_label(context);
        Widget::draw(std::move(context), display_time_point);
    }

    bool handle_mouse_event(MouseEvent const &event) noexcept override
    {
        ttlet lock = std::scoped_lock(GUISystem_mutex);
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
        ttlet lock = std::scoped_lock(GUISystem_mutex);
        auto handled = Widget::handle_command(command);

        if (*enabled) {
            if (command == command::gui_activate) {
                if (compare_then_assign(value, ActiveValue)) {
                    window.requestRedraw = true;
                }
            }
        }
        return handled;
    }

    [[nodiscard]] HitBox hitbox_test(vec window_position) const noexcept override
    {
        ttlet lock = std::scoped_lock(GUISystem_mutex);

        if (p_window_clipping_rectangle.contains(window_position)) {
            return HitBox{weak_from_this(), p_draw_layer, *enabled ? HitBox::Type::Button : HitBox::Type::Default};
        } else {
            return HitBox{};
        }
    }

    [[nodiscard]] bool accepts_focus() const noexcept override
    {
        tt_assume(GUISystem_mutex.recurse_lock_count());
        return *enabled;
    }

private:
    aarect radioButtonRectangle;
    aarect pipRectangle;
    aarect _label_rectangle;

    std::unique_ptr<TextCell> labelCell;

    typename decltype(value)::callback_ptr_type value_callback;
    typename decltype(label)::callback_ptr_type label_callback;

    void drawRadioButton(DrawContext drawContext) noexcept
    {
        tt_assume(GUISystem_mutex.recurse_lock_count());

        drawContext.cornerShapes = vec{radioButtonRectangle.height() * 0.5f};
        drawContext.drawBoxIncludeBorder(radioButtonRectangle);
    }

    void drawPip(DrawContext drawContext) noexcept
    {
        tt_assume(GUISystem_mutex.recurse_lock_count());

        // draw pip
        if (value == ActiveValue) {
            if (*enabled && window.active) {
                drawContext.color = theme->accentColor;
            }
            std::swap(drawContext.color, drawContext.fillColor);
            drawContext.cornerShapes = vec{pipRectangle.height() * 0.5f};
            drawContext.drawBoxIncludeBorder(pipRectangle);
        }
    }

    void draw_label(DrawContext drawContext) noexcept
    {
        tt_assume(GUISystem_mutex.recurse_lock_count());

        if (*enabled) {
            drawContext.color = theme->labelStyle.color;
        }

        labelCell->draw(drawContext, _label_rectangle, Alignment::TopLeft, base_line(), true);
    }
};

} // namespace tt
