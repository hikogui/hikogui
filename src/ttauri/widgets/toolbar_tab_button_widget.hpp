// Copyright 2020 Pokitec
// All rights reserved.

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
        std::shared_ptr<widget> parent,
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

    /** The tab button widget will draw beyond the normal clipping rectangle.
     */
    [[nodiscard]] aarect window_clipping_rectangle() const noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        ttlet parent_ = this->parent.lock();
        return parent_->window_clipping_rectangle();
    }

    [[nodiscard]] bool update_constraints(hires_utc_clock::time_point display_time_point, bool need_reconstrain) noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        if (super::update_constraints(display_time_point, need_reconstrain)) {
            _label_stencil = stencil::make_unique(alignment::top_center, *label, theme::global->labelStyle);

            ttlet minimum_height = _label_stencil->preferred_extent().height();
            ttlet minimum_width = _label_stencil->preferred_extent().width() + 2.0f * theme::global->margin;

            this->_preferred_size = {
                f32x4{minimum_width, minimum_height},
                f32x4{std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity()}};
            this->_preferred_base_line = relative_base_line{vertical_alignment::middle, -theme::global->margin};
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
            this->window.request_redraw(this->window_clipping_rectangle());

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

        if (overlaps(context, this->window_clipping_rectangle())) {
            draw_button(context);
            draw_label(context);
            draw_focus_line(context);
        }

        super::draw(std::move(context), display_time_point);
    }

private:
    typename decltype(label)::callback_ptr_type _label_callback;
    aarect _button_rectangle;
    std::unique_ptr<stencil> _label_stencil;

    void draw_focus_line(draw_context const &context) noexcept
    {
        if (this->_focus && this->window.active && *this->value == this->true_value) {
            ttlet parent_ = this->parent.lock();

            // Draw the focus line over the full width of the window at the bottom
            // of the toolbar.
            auto parent_context = parent_->make_draw_context(context);

            // Draw the line above every other direct child of the toolbar, and between
            // the selected-tab (0.6) and unselected-tabs (0.8).
            parent_context.transform = mat::T(0.0f, 0.0f, 1.7f) * parent_context.transform;

            parent_context.fill_color = theme::global->accentColor;
            parent_context.draw_filled_quad(
                aarect{parent_->rectangle().x(), parent_->rectangle().y(), parent_->rectangle().width(), 1.0f});
        }
    }

    void draw_button(draw_context context) noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());
        if (this->_focus && this->window.active) {
            // The focus line will be placed at 0.7.
            context.transform = mat::T(0.0f, 0.0f, 0.8f) * context.transform;
        } else {
            context.transform = mat::T(0.0f, 0.0f, 0.6f) * context.transform;
        }

        // Override the clipping rectangle to match the toolbar rectangle exactly
        // so that the bottom border of the tab button is not drawn.
        ttlet parent_ = this->parent.lock();
        context.clipping_rectangle = parent_->window_rectangle();

        if (this->_hover || *this->value == this->true_value) {
            context.fill_color = theme::global->fillColor(this->_semantic_layer - 1);
            context.color = context.fill_color;
        } else {
            context.fill_color = theme::global->fillColor(this->_semantic_layer);
            context.color = context.fill_color;
        }

        if (this->_focus && this->window.active) {
            context.color = theme::global->accentColor;
        }

        context.corner_shapes = f32x4{0.0f, 0.0f, theme::global->roundingRadius, theme::global->roundingRadius};
        context.draw_box_with_border_inside(_button_rectangle);
    }

    void draw_label(draw_context context) noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        context.transform = mat::T(0.0f, 0.0f, 0.9f) * context.transform;

        if (*this->enabled) {
            context.color = theme::global->labelStyle.color;
        }

        _label_stencil->draw(context, true);
    }
};

} // namespace tt
