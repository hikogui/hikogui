// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "widget.hpp"
#include "label_widget.hpp"
#include "overlay_view_widget.hpp"
#include "scroll_view_widget.hpp"
#include "row_column_layout_widget.hpp"
#include "menu_button_widget.hpp"
#include "selection_delegate.hpp"
#include "value_selection_delegate.hpp"
#include "../GUI/draw_context.hpp"
#include "../text/font_book.hpp"
#include "../text/elusive_icon.hpp"
#include "../observable.hpp"
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>

namespace tt {

class selection_widget final : public widget {
public:
    using super = widget;
    using delegate_type = selection_delegate;

    observable<label> unknown_label;

    selection_widget(
        gui_window &window,
        std::shared_ptr<widget> parent, std::shared_ptr<delegate_type> delegate) noexcept :
        super(window, parent),
        _delegate(std::move(delegate))
    {
    }

    template<typename Label, typename... DelegateArgs>
    selection_widget(gui_window &window,
        std::shared_ptr<widget> parent,
        Label &&label, DelegateArgs &&... delegate_args) noexcept :
        selection_widget(window, std::move(parent), make_value_selection_delegate(std::forward<DelegateArgs>(delegate_args)...))
    {
        unknown_label = std::forward<Label>(label);
    }

    ~selection_widget() {}

    void init() noexcept override
    {
        super::init();

        _current_label_widget = make_widget<label_widget>(l10n("<current>"), alignment::middle_left, theme::global().label_style);
        _current_label_widget->visible = false;
        _unknown_label_widget =
            make_widget<label_widget>(unknown_label, alignment::middle_left, theme::global().placeholder_label_style);

        _overlay_widget = make_widget<overlay_view_widget>();
        _overlay_widget->visible = false;
        _scroll_widget = _overlay_widget->make_widget<vertical_scroll_view_widget<>>();
        _column_widget = _scroll_widget->make_widget<column_layout_widget>();

        _unknown_label_callback = this->unknown_label.subscribe([this](auto...) {
            ttlet lock = std::scoped_lock(gui_system_mutex);

            _request_reconstrain = true;
        });

        _delegate_callback = _delegate->subscribe(*this, [this](auto...) {
            ttlet lock = std::scoped_lock(gui_system_mutex);

            repopulate_options();
            _request_reconstrain = true;
        });

        (*_delegate_callback)();

        _delegate->init(*this);
    }

    void deinit() noexcept override
    {
        _delegate->deinit(*this);
        super::deinit();
    }

    [[nodiscard]] bool update_constraints(hires_utc_clock::time_point display_time_point, bool need_reconstrain) noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        auto updated = super::update_constraints(display_time_point, need_reconstrain);
        if (updated) {
            ttlet extra_size = extent2{theme::global().size + theme::global().margin * 2.0f, theme::global().margin * 2.0f};

            _minimum_size = _unknown_label_widget->minimum_size() + extra_size;
            _preferred_size = _unknown_label_widget->preferred_size() + extra_size;
            _maximum_size = _unknown_label_widget->maximum_size() + extra_size;

            _minimum_size = max(_minimum_size, _current_label_widget->minimum_size() + extra_size);
            _preferred_size = max(_preferred_size, _current_label_widget->preferred_size() + extra_size);
            _maximum_size = max(_maximum_size, _current_label_widget->maximum_size() + extra_size);

            for (ttlet &child : _menu_button_widgets) {
                _minimum_size = max(_minimum_size, child->minimum_size());
                _preferred_size = max(_preferred_size, child->preferred_size());
                _maximum_size = max(_maximum_size, child->maximum_size());
            }

            _minimum_size.width() = std::max(_minimum_size.width(), _overlay_widget->minimum_size().width() + extra_size.width());
            _preferred_size.width() =
                std::max(_preferred_size.width(), _overlay_widget->preferred_size().width() + extra_size.width());
            _maximum_size.width() = std::max(_maximum_size.width(), _overlay_widget->maximum_size().width() + extra_size.width());

            tt_axiom(_minimum_size <= _preferred_size && _preferred_size <= _maximum_size);
            return true;

        } else {
            return false;
        }
    }

    [[nodiscard]] void update_layout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        need_layout |= std::exchange(_request_relayout, false);

        if (need_layout) {
            // The overlay itself will make sure the overlay fits the window, so we give the preferred size and position
            // from the point of view of the selection widget.

            // The overlay should start on the same left edge as the selection box and the same width.
            // The height of the overlay should be the maximum height, which will show all the options.

            ttlet overlay_width = std::clamp(
                rectangle().width() - theme::global().size,
                _overlay_widget->minimum_size().width(),
                _overlay_widget->maximum_size().width());
            ttlet overlay_height = _overlay_widget->preferred_size().height();
            ttlet overlay_x = theme::global().size;
            ttlet overlay_y = std::round(_size.height() * 0.5f - overlay_height * 0.5f);
            ttlet overlay_rectangle_request = aarectangle{overlay_x, overlay_y, overlay_width, overlay_height};

            ttlet overlay_rectangle = _overlay_widget->make_overlay_rectangle_from_parent(overlay_rectangle_request);
            ttlet overlay_clipping_rectangle = expand(overlay_rectangle, _overlay_widget->margin());

            _overlay_widget->set_layout_parameters_from_parent(
                overlay_rectangle, overlay_clipping_rectangle, _overlay_widget->draw_layer() - _draw_layer);

            _left_box_rectangle = aarectangle{0.0f, 0.0f, theme::global().size, rectangle().height()};
            _chevrons_glyph = to_font_glyph_ids(elusive_icon::ChevronUp);
            ttlet chevrons_glyph_bbox = pipeline_SDF::device_shared::getBoundingBox(_chevrons_glyph);
            _chevrons_rectangle =
                align(_left_box_rectangle, scale(chevrons_glyph_bbox, theme::global().icon_size), alignment::middle_center);
            _chevrons_rectangle =
                align(_left_box_rectangle, scale(chevrons_glyph_bbox, theme::global().icon_size), alignment::middle_center);

            // The unknown_label is located to the right of the selection box icon.
            _option_rectangle = aarectangle{
                _left_box_rectangle.right() + theme::global().margin,
                0.0f,
                rectangle().width() - _left_box_rectangle.width() - theme::global().margin * 2.0f,
                rectangle().height()};

            _unknown_label_widget->set_layout_parameters_from_parent(_option_rectangle);
            _current_label_widget->set_layout_parameters_from_parent(_option_rectangle);
        }
        super::update_layout(display_time_point, need_layout);
    }

    void draw(draw_context context, hires_utc_clock::time_point display_time_point) noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        if (overlaps(context, this->_clipping_rectangle)) {
            draw_outline(context);
            draw_left_box(context);
            draw_chevrons(context);
        }

        super::draw(std::move(context), display_time_point);
    }

    bool handle_event(mouse_event const &event) noexcept override
    {
        ttlet lock = std::scoped_lock(gui_system_mutex);
        auto handled = super::handle_event(event);

        if (event.cause.leftButton) {
            handled = true;
            if (enabled and _has_options) {
                if (event.type == mouse_event::Type::ButtonUp && rectangle().contains(event.position)) {
                    handle_event(command::gui_activate);
                }
            }
        }
        return handled;
    }

    bool handle_event(command command) noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());
        _request_relayout = true;

        if (enabled and _has_options) {
            switch (command) {
                using enum tt::command;
            case gui_activate:
            case gui_enter:
                if (!_selecting) {
                    start_selecting();
                } else {
                    stop_selecting();
                }
                return true;

            case gui_escape:
                if (_selecting) {
                    stop_selecting();
                }
                return true;

            default:;
            }
        }

        return super::handle_event(command);
    }

    [[nodiscard]] hit_box hitbox_test(point2 position) const noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        auto r = super::hitbox_test(position);
        if (_visible_rectangle.contains(position)) {
            r = std::max(r, hit_box{weak_from_this(), _draw_layer, (enabled and _has_options) ? hit_box::Type::Button : hit_box::Type::Default});
        }

        return r;
    }

    [[nodiscard]] bool accepts_keyboard_focus(keyboard_focus_group group) const noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());
        return is_normal(group) and enabled and _has_options;
    }

    [[nodiscard]] color focus_color() const noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        if (enabled and _has_options and _selecting) {
            return theme::global(theme_color::accent);
        } else {
            return super::focus_color();
        }
    }

private:
    std::shared_ptr<delegate_type> _delegate;

    typename delegate_type::callback_ptr_type _delegate_callback;
    typename decltype(unknown_label)::callback_ptr_type _unknown_label_callback;

    std::shared_ptr<label_widget> _current_label_widget;
    std::shared_ptr<label_widget> _unknown_label_widget;

    aarectangle _option_rectangle;
    aarectangle _left_box_rectangle;

    font_glyph_ids _chevrons_glyph;
    aarectangle _chevrons_rectangle;

    bool _selecting = false;
    bool _has_options = false;

    std::shared_ptr<overlay_view_widget> _overlay_widget;
    std::shared_ptr<vertical_scroll_view_widget<>> _scroll_widget;
    std::shared_ptr<column_layout_widget> _column_widget;

    std::vector<std::shared_ptr<menu_button_widget>> _menu_button_widgets;
    std::vector<typename menu_button_widget::callback_ptr_type> _menu_button_callbacks;

    [[nodiscard]] std::shared_ptr<menu_button_widget> get_first_menu_button() const noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        if (std::ssize(_menu_button_widgets) != 0) {
            return _menu_button_widgets.front();
        } else {
            return {};
        }
    }

    [[nodiscard]] std::shared_ptr<menu_button_widget> get_selected_menu_button() const noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        for (ttlet &button: _menu_button_widgets) {
            if (button->state() == button_state::on) {
                return button;
            }
        }
        return {};
    }

    void start_selecting() noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        _selecting = true;
        _overlay_widget->visible = true;
        if (auto selected_menu_button = get_selected_menu_button()) {
            this->window.update_keyboard_target(selected_menu_button, keyboard_focus_group::menu);

        } else if (auto first_menu_button = get_first_menu_button()) {
            this->window.update_keyboard_target(first_menu_button, keyboard_focus_group::menu);
        }

        request_redraw();
    }

    void stop_selecting() noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());
        _selecting = false;
        _overlay_widget->visible = false;
        request_redraw();
    }

    /** Populate the scroll view with menu items corresponding to the options.
     */
    void repopulate_options() noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());
        auto [options, selected] = _delegate->options_and_selected(*this);

        _has_options = std::size(options) > 0;

        // If any of the options has a an icon, all of the options should show the icon.
        auto show_icon = false;
        for (ttlet &label : options) {
            show_icon |= static_cast<bool>(label.icon);
        }

        _column_widget->clear();
        _menu_button_widgets.clear();
        _menu_button_callbacks.clear();

        decltype(selected) index = 0;
        for (auto &&label : options) {
            auto menu_button = _column_widget->make_widget<menu_button_widget>(std::move(label), selected, index);

            _menu_button_callbacks.push_back(menu_button->subscribe([this, index] {
                ttlet lock = std::scoped_lock(gui_system_mutex);
                this->_delegate->set_selected(*this, index);
                this->stop_selecting();
            }));

            _menu_button_widgets.push_back(std::move(menu_button));

            ++index;
        }

        if (selected == -1) {
            _unknown_label_widget->visible = true;
            _current_label_widget->visible = false;

        } else {
            _unknown_label_widget->visible = false;
            _current_label_widget->label = options[selected];
            _current_label_widget->visible = true;
        }
    }

    void draw_outline(draw_context context) noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        context.draw_box_with_border_inside(
            rectangle(), background_color(), focus_color(), corner_shapes{theme::global().rounding_radius});
    }

    void draw_left_box(draw_context context) noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        ttlet corner_shapes = tt::corner_shapes{theme::global().rounding_radius, 0.0f, theme::global().rounding_radius, 0.0f};
        context.draw_box(translate_z(0.1f) * _left_box_rectangle, focus_color(), corner_shapes);
    }

    void draw_chevrons(draw_context context) noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        context.draw_glyph(_chevrons_glyph, translate_z(0.2f) * _chevrons_rectangle, label_color());
    }
};

} // namespace tt
