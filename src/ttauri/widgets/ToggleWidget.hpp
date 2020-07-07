// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Widgets/Widget.hpp"
#include "TTauri/Cells/TextCell.hpp"
#include "TTauri/GUI/DrawContext.hpp"
#include "TTauri/Foundation/observable.hpp"
#include "TTauri/Text/format10.hpp"
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>

namespace tt {

template<typename ValueType=bool>
class ToggleWidget : public Widget {
protected:
    static constexpr hires_utc_clock::duration animation_duration = 150ms;

    float toggle_height;
    float toggle_width;
    float toggle_x;
    float toggle_y;
    float toggle_middle;
    aarect toggle_rectangle;

    float slider_x;
    float slider_y;
    float slider_move;
    float slider_width;
    float slider_height;

    aarect label_rectangle;
    mat::T label_translate;
    std::unique_ptr<TextCell> labelCell;

    ValueType trueValue;

public:
    observable<ValueType> value;
    observable<std::string> label;

    template<typename V>
    ToggleWidget(Window &window, Widget *parent, V &&value, ValueType trueValue) noexcept :
        Widget(window, parent, vec{Theme::smallWidth, Theme::smallHeight}),
        value(std::forward<V>(value)),
        label()
    {
        [[maybe_unused]] ttlet value_cbid = this->value.add_callback([this](auto...){
            forceRedraw = true;
        });
        [[maybe_unused]] ttlet label_cbid = this->label.add_callback([this](auto...) {
            forceLayout = true;
        });
    }

    ~ToggleWidget() {
    }

    ToggleWidget(const ToggleWidget &) = delete;
    ToggleWidget &operator=(const ToggleWidget &) = delete;
    ToggleWidget(ToggleWidget&&) = delete;
    ToggleWidget &operator=(ToggleWidget &&) = delete;

    void layout(hires_utc_clock::time_point displayTimePoint) noexcept override {
        Widget::layout(displayTimePoint);

        // The label is located to the right of the toggle.
        ttlet label_x = Theme::smallWidth + Theme::margin;
        label_rectangle = aarect{
            label_x, 0.0f,
            rectangle().width() - label_x, rectangle().height()
        };

        labelCell = std::make_unique<TextCell>(*label, theme->labelStyle);
        setFixedHeight(std::max(labelCell->heightForWidth(label_rectangle.width()), Theme::smallHeight));

        toggle_height = Theme::smallHeight;
        toggle_width = Theme::smallWidth + 1.0f; // Expand horizontally due to rounded shape
        toggle_x = -0.5f;  // Expand horizontally due to rounded shape
        toggle_y = rectangle().height() - toggle_height;
        toggle_rectangle = aarect{toggle_x, toggle_y, toggle_width, toggle_height};
        toggle_middle = toggle_y + toggle_height * 0.5f;

        slider_x = 1.5f;
        slider_y = toggle_y + 1.5f;
        slider_width = toggle_height - 3.0f;
        slider_height = toggle_height - 3.0f;
        ttlet slider_move_width = Theme::smallWidth - (slider_x * 2.0f);
        slider_move = slider_move_width - slider_width;
    }
    
    void draw(DrawContext const &drawContext, hires_utc_clock::time_point displayTimePoint) noexcept override {
        // Prepare animation values.
        ttlet animation_progress = value.animation_progress(animation_duration);
        if (animation_progress < 1.0f) {
            forceRedraw = true;
        }

        ttlet animated_value = to_float(value, animation_duration);

        // Outside oval.
        auto context = drawContext;
        context.cornerShapes = vec{toggle_rectangle.height() * 0.5f};
        context.drawBoxIncludeBorder(toggle_rectangle);

        // Inside circle
        ttlet slider_rectangle = aarect{
            slider_x + slider_move * animated_value, slider_y,
            slider_width, slider_height
        };

        if (value == trueValue) {
            if (*enabled && window.active) {
                context.color = theme->accentColor;
            }
        } else {
            if (*enabled && window.active) {
                context.color = hover ?
                    theme->borderColor(nestingLevel() + 1) :
                    theme->borderColor(nestingLevel());
            }
        }
        std::swap(context.color, context.fillColor);
        context.cornerShapes = vec{slider_rectangle.height() * 0.5f};
        context.drawBoxIncludeBorder(slider_rectangle);

        labelCell->draw(context, label_rectangle, Alignment::TopLeft, toggle_middle);
        Widget::draw(drawContext, displayTimePoint);
    }

    void handleMouseEvent(MouseEvent const &event) noexcept override {
        Widget::handleMouseEvent(event);

        if (*enabled) {
            if (
                event.type == MouseEvent::Type::ButtonUp &&
                event.cause.leftButton &&
                rectangle().contains(event.position)
                ) {
                handleCommand("gui.activate"_ltag);
            }
        }
    }
    
    void handleCommand(string_ltag command) noexcept override {
        if (!*enabled) {
            return;
        }

        if (command == "gui.activate"_ltag) {
            if (assign_and_compare(value, !*value)) {
                forceRedraw = true;
            }
        }
        Widget::handleCommand(command);
    }

    HitBox hitBoxTest(vec position) const noexcept override {
        if (rectangle().contains(position)) {
            return HitBox{this, elevation, *enabled ? HitBox::Type::Button : HitBox::Type::Default};
        } else {
            return HitBox{};
        }
    }

    [[nodiscard]] bool acceptsFocus() const noexcept override {
        return *enabled;
    }

};

}
