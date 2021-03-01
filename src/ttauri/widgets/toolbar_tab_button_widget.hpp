// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "abstract_radio_button_widget.hpp"
#include "../GUI/draw_context.hpp"
#include "../observable.hpp"
#include "../label.hpp"
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>

namespace tt {

template<typename T>
class toolbar_tab_button_widget final : public abstract_radio_button_widget<T> {
public:
    using super = abstract_radio_button_widget<T>;
    using value_type = super::value_type;

    observable<label> label;

    template<typename Value, typename Label = observable<tt::label>>
    toolbar_tab_button_widget(
        gui_window &window,
        std::shared_ptr<abstract_container_widget> parent,
        value_type true_value,
        Value &&value,
        Label &&label = {}) noexcept :
        abstract_radio_button_widget<T>(window, parent, std::move(true_value), std::forward<Value>(value)),
        label(std::forward<Label>(label))
    {
        this->_width_resistance = 2;
    }

    toolbar_tab_button_widget(gui_window &window, std::shared_ptr<widget> parent, value_type true_value) noexcept :
        toolbar_tab_button_widget(window, parent, std::move(true_value), observable<int>{}, observable<tt::label>{})
    {
    }

    ~toolbar_tab_button_widget() {}

    void init() noexcept override
    {
        _label_callback = label.subscribe([this](auto...) {
            this->_request_reconstrain = true;
        });
    }

    [[nodiscard]] bool accepts_keyboard_focus(keyboard_focus_group group) const noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());
        return is_toolbar(group) && *this->enabled;
    }

    [[nodiscard]] bool update_constraints(hires_utc_clock::time_point display_time_point, bool need_reconstrain) noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        if (super::update_constraints(display_time_point, need_reconstrain)) {
            _label_stencil = stencil::make_unique(alignment::top_center, *label, theme::global->labelStyle);

            ttlet minimum_height = _label_stencil->preferred_extent().height();
            ttlet minimum_width = _label_stencil->preferred_extent().width() + 2.0f * theme::global->margin;

            this->_preferred_size = {
                extent2{minimum_width, minimum_height},
                extent2{std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity()}};
            return true;
        } else {
            return false;
        }
    }

    [[nodiscard]] void update_layout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        need_layout |= std::exchange(this->_request_relayout, false);
        if (need_layout) {
            // A tab button widget draws beyond its clipping rectangle.
            this->window.request_redraw(aarect{this->_local_to_window * this->_clipping_rectangle});

            ttlet offset = theme::global->margin + theme::global->borderWidth;
            _button_rectangle = aarect{
                this->rectangle().x(),
                this->rectangle().y() - offset,
                this->rectangle().width(),
                this->rectangle().height() + offset};

            _label_stencil->set_layout_parameters(this->rectangle(), this->base_line());
        }

        super::update_layout(display_time_point, need_layout);
    }

    void draw(draw_context context, hires_utc_clock::time_point display_time_point) noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        if (overlaps(context, this->_clipping_rectangle)) {
            draw_button(context);
            draw_label(context);
            draw_focus_line(context);
        }

        super::draw(std::move(context), display_time_point);
    }

    [[nodiscard]] bool handle_event(tt::command command) noexcept override
    {
        switch (command) {
        case command::gui_toolbar_next:
            if (!this->is_last(keyboard_focus_group::toolbar)) {
                this->window.update_keyboard_target(
                    this->shared_from_this(), keyboard_focus_group::toolbar, keyboard_focus_direction::forward);
            }
            return true;

        case command::gui_toolbar_prev:
            if (!this->is_first(keyboard_focus_group::toolbar)) {
                this->window.update_keyboard_target(
                    this->shared_from_this(), keyboard_focus_group::toolbar, keyboard_focus_direction::backward);
            }
            return true;

        default:;
        }

        return super::handle_event(command);
    }

private:
    typename decltype(label)::callback_ptr_type _label_callback;
    aarect _button_rectangle;
    std::unique_ptr<stencil> _label_stencil;

    void draw_focus_line(draw_context const &context) noexcept
    {
        if (this->_focus && this->window.active && *this->value == this->true_value) {
            ttlet &parent_ = this->parent();

            // Draw the focus line over the full width of the window at the bottom
            // of the toolbar.
            auto parent_context = parent_.make_draw_context(context);

            ttlet line_rectangle = aarect{extent2{parent_.rectangle().width(), 1.0f}};
            
            // Draw the line above every other direct child of the toolbar, and between
            // the selected-tab (0.6) and unselected-tabs (0.8).
            parent_context.draw_filled_quad(translate_z(1.7f) * line_rectangle, this->focus_color());
        }
    }

    void draw_button(draw_context context) noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        // Override the clipping rectangle to match the toolbar rectangle exactly
        // so that the bottom border of the tab button is not drawn.
        context.set_clipping_rectangle(aarect{this->_parent_to_local * this->parent().clipping_rectangle()});

        // The focus line will be placed at 0.7.
        ttlet button_z = (this->_focus && this->window.active) ? translate_z(0.8f) : translate_z(0.6f);

        auto button_color = (this->_hover || *this->value == this->true_value) ?
            theme::global->fillColor(this->_semantic_layer - 1) :
            theme::global->fillColor(this->_semantic_layer);

        ttlet corner_shapes = tt::corner_shapes{0.0f, 0.0f, theme::global->roundingRadius, theme::global->roundingRadius};
        context.draw_box_with_border_inside(
            button_z * _button_rectangle,
            button_color,
            (this->_focus && this->window.active) ? this->focus_color() : button_color,
            corner_shapes);
    }

    void draw_label(draw_context context) noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());
        _label_stencil->draw(context, this->label_color(), translate_z(0.9f));
    }
};

} // namespace tt
