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
    using value_type = typename super::value_type;

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
            ttlet extra_size = extent2{0.0f, theme::global->margin * 2.0f};

            this->_minimum_size = _label_stencil->minimum_size() + extra_size;
            this->_preferred_size = _label_stencil->preferred_size() + extra_size;
            this->_maximum_size = _label_stencil->maximum_size() + extra_size;
            tt_axiom(this->_minimum_size <= this->_preferred_size && this->_preferred_size <= this->_maximum_size);
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
            this->request_redraw();

            ttlet offset = theme::global->margin + theme::global->borderWidth;
            _button_rectangle = aarectangle{
                this->rectangle().left(),
                this->rectangle().bottom() - offset,
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
        }

        draw_focus_line(context);

        super::draw(std::move(context), display_time_point);
    }

    [[nodiscard]] bool handle_event(tt::command command) noexcept override
    {
        switch (command) {
        case command::gui_toolbar_next:
            if (!this->is_last(keyboard_focus_group::toolbar)) {
                this->window.update_keyboard_target(keyboard_focus_group::toolbar, keyboard_focus_direction::forward);
            }
            return true;

        case command::gui_toolbar_prev:
            if (!this->is_first(keyboard_focus_group::toolbar)) {
                this->window.update_keyboard_target(keyboard_focus_group::toolbar, keyboard_focus_direction::backward);
            }
            return true;

        default:;
        }

        auto r = super::handle_event(command);
        if (r) {
            // Let the toolbar request a redraw, so that the extended focus line get redrawn when it changes.
            auto parent = this->_parent.lock();
            tt_axiom(parent);
            parent->request_redraw();
        }
        return r;
    }

private:
    typename decltype(label)::callback_ptr_type _label_callback;
    aarectangle _button_rectangle;
    std::unique_ptr<stencil> _label_stencil;

    void draw_focus_line(draw_context context) noexcept
    {
        if (this->_focus && this->window.active && *this->value == this->true_value) {
            ttlet &parent_ = this->parent();
            ttlet parent_rectangle = aarectangle{this->_parent_to_local * parent_.rectangle()};

            // Create a line, on the bottom of the toolbar over the full width.
            ttlet line_rectangle = aarectangle{
                parent_rectangle.left(),
                parent_rectangle.bottom(),
                parent_rectangle.width(),
                theme::global->borderWidth
            };

            context.set_clipping_rectangle(line_rectangle);

            if (overlaps(context, line_rectangle)) {
                // Draw the line above every other direct child of the toolbar, and between
                // the selected-tab (0.6) and unselected-tabs (0.8).
                context.draw_filled_quad(translate_z(0.7f) * line_rectangle, this->focus_color());
            }
        }
    }

    void draw_button(draw_context context) noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        // Override the clipping rectangle to match the toolbar rectangle exactly
        // so that the bottom border of the tab button is not drawn.
        context.set_clipping_rectangle(aarectangle{this->_parent_to_local * this->parent().clipping_rectangle()});

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
