// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "abstract_container_widget.hpp"
#include "overlay_view_widget.hpp"
#include "scroll_view_widget.hpp"
#include "row_column_layout_widget.hpp"
#include "menu_item_widget.hpp"
#include "../stencils/label_stencil.hpp"
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

template<typename T>
class selection_widget final : public abstract_container_widget {
public:
    using super = abstract_container_widget;
    using value_type = T;
    using option_list_type = std::vector<std::pair<value_type, label>>;

    observable<label> unknown_label;
    observable<value_type> value;
    observable<option_list_type> option_list;

    template<typename Value = value_type, typename OptionList = option_list_type, typename UnknownLabel = label>
    selection_widget(
        gui_window &window,
        std::shared_ptr<abstract_container_widget> parent,
        Value &&value = value_type{},
        OptionList &&option_list = option_list_type{},
        UnknownLabel &&unknown_label = label{l10n("<unknown>")}) noexcept :
        super(window, parent),
        value(std::forward<Value>(value)),
        option_list(std::forward<OptionList>(option_list)),
        unknown_label(std::forward<UnknownLabel>(unknown_label))
    {
        // Because the super class `abstract_container_widget` forces the same semantic layer
        // as the a parent, we need to force it back as if this is a normal widget.
        _semantic_layer = parent->semantic_layer() + 1;
    }

    ~selection_widget() {}

    void init() noexcept override
    {
        _overlay_widget = super::make_widget<overlay_view_widget>();
        _scroll_widget = _overlay_widget->make_widget<vertical_scroll_view_widget<>>();
        _column_widget = _scroll_widget->make_widget<column_layout_widget>();

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

    [[nodiscard]] bool update_constraints(hires_utc_clock::time_point display_time_point, bool need_reconstrain) noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        auto updated = super::update_constraints(display_time_point, need_reconstrain);

        if (updated) {
            ttlet index = get_value_as_index();
            if (index == -1) {
                _text_stencil =
                    stencil::make_unique(alignment::middle_left, *unknown_label, theme::global->placeholderLabelStyle);
                _text_stencil_color = theme::global->placeholderLabelStyle.color;
            } else {
                _text_stencil =
                    stencil::make_unique(alignment::middle_left, (*option_list)[index].second, theme::global->labelStyle);
                _text_stencil_color = theme::global->labelStyle.color;
            }

            // Calculate the size of the widget based on the largest height of a label and the width of the overlay.
            ttlet unknown_label_size =
                stencil::make_unique(alignment::middle_left, *unknown_label, theme::global->placeholderLabelStyle)
                    ->preferred_extent();

            ttlet overlay_width = _overlay_widget->preferred_size().minimum().width();
            ttlet option_width = std::max(overlay_width, unknown_label_size.width() + theme::global->margin * 2.0f);
            ttlet option_height = std::max(unknown_label_size.height(), _max_option_label_height) + theme::global->margin * 2.0f;
            ttlet chevron_width = theme::global->smallSize;

            _preferred_size = interval_extent2::make_minimum(extent2{chevron_width + option_width, option_height});
            return true;

        } else {
            return false;
        }
    }

    [[nodiscard]] void update_layout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        need_layout |= std::exchange(_request_relayout, false);

        if (_selecting) {
            if (need_layout) {
                // The overlay itself will make sure the overlay fits the window, so we give the preferred size and position
                // from the point of view of the selection widget.

                // The overlay should start on the same left edge as the selection box and the same width.
                // The height of the overlay should be the maximum height, which will show all the options.

                ttlet overlay_width =
                    clamp(rectangle().width() - theme::global->smallSize, _overlay_widget->preferred_size().width());
                ttlet overlay_height = _overlay_widget->preferred_size().maximum().height();
                ttlet overlay_x = theme::global->smallSize;
                ttlet overlay_y = std::round(_size.height() * 0.5f - overlay_height * 0.5f);
                ttlet overlay_rectangle_request = aarect{overlay_x, overlay_y, overlay_width, overlay_height};

                ttlet overlay_rectangle = _overlay_widget->make_overlay_rectangle_from_parent(overlay_rectangle_request);
                ttlet overlay_clipping_rectangle = expand(overlay_rectangle, _overlay_widget->margin());

                _overlay_widget->set_layout_parameters_from_parent(
                    overlay_rectangle, overlay_clipping_rectangle, _overlay_widget->draw_layer() - _draw_layer);
            }
        }

        if (need_layout) {
            _left_box_rectangle = aarect{0.0f, 0.0f, theme::global->smallSize, rectangle().height()};
            _chevrons_glyph = to_font_glyph_ids(elusive_icon::ChevronUp);
            ttlet chevrons_glyph_bbox = pipeline_SDF::device_shared::getBoundingBox(_chevrons_glyph);
            _chevrons_rectangle =
                align(_left_box_rectangle, scale(chevrons_glyph_bbox, theme::global->small_icon_size), alignment::middle_center);
            _chevrons_rectangle =
                align(_left_box_rectangle, scale(chevrons_glyph_bbox, theme::global->small_icon_size), alignment::middle_center);

            // The unknown_label is located to the right of the selection box icon.
            _option_rectangle = aarect{
                _left_box_rectangle.right() + theme::global->margin,
                0.0f,
                rectangle().width() - _left_box_rectangle.width() - theme::global->margin * 2.0f,
                rectangle().height()};

            _text_stencil->set_layout_parameters(_option_rectangle, base_line());
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
            draw_value(context);
        }

        if (_selecting) {
            super::draw(std::move(context), display_time_point);
        }
    }

    bool handle_event(mouse_event const &event) noexcept override
    {
        ttlet lock = std::scoped_lock(gui_system_mutex);
        auto handled = super::handle_event(event);

        if (event.cause.leftButton) {
            handled = true;
            if (*enabled) {
                if (event.type == mouse_event::Type::ButtonUp && rectangle().contains(event.position)) {
                    handle_event(command::gui_activate);
                }
            }
        }
        return handled;
    }

    bool handle_event(command command) noexcept override
    {
        ttlet lock = std::scoped_lock(gui_system_mutex);
        _request_relayout = true;

        if (*enabled) {
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
        ttlet lock = std::scoped_lock(gui_system_mutex);

        auto r = hit_box{};

        if (_selecting) {
            r = super::hitbox_test(position);
        }

        if (rectangle().contains(position)) {
            r = std::max(r, hit_box{weak_from_this(), _draw_layer, *enabled ? hit_box::Type::Button : hit_box::Type::Default});
        }

        return r;
    }

    [[nodiscard]] bool accepts_keyboard_focus(keyboard_focus_group group) const noexcept override
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());
        return is_normal(group) && *enabled;
    }

    std::shared_ptr<widget> find_next_widget(
        std::shared_ptr<widget> const &current_widget,
        keyboard_focus_group group,
        keyboard_focus_direction direction) const noexcept
    {
        ttlet lock = std::scoped_lock(gui_system_mutex);

        if (_selecting) {
            return super::find_next_widget(current_widget, group, direction);

        } else {
            // Bypass the abstract_container_widget and directly use the widget implementation.
            return widget::find_next_widget(current_widget, group, direction);
        }
    }

    template<typename T, typename... Args>
    std::shared_ptr<T> make_widget(Args &&...args)
    {
        tt_no_default();
    }

    [[nodiscard]] color focus_color() const noexcept override
    {
        if (*enabled && _selecting) {
            return theme::global->accentColor;
        } else {
            return super::focus_color();
        }
    }

private:
    typename decltype(unknown_label)::callback_ptr_type _unknown_label_callback;
    typename decltype(value)::callback_ptr_type _value_callback;
    typename decltype(option_list)::callback_ptr_type _option_list_callback;

    std::unique_ptr<label_stencil> _text_stencil;
    color _text_stencil_color;

    float _max_option_label_height;

    aarect _option_rectangle;
    aarect _left_box_rectangle;

    font_glyph_ids _chevrons_glyph;
    aarect _chevrons_rectangle;

    bool _selecting = false;
    std::shared_ptr<overlay_view_widget> _overlay_widget;
    std::shared_ptr<vertical_scroll_view_widget<>> _scroll_widget;
    std::shared_ptr<column_layout_widget> _column_widget;

    std::vector<std::shared_ptr<menu_item_widget<value_type>>> _menu_item_widgets;
    std::vector<typename menu_item_widget<value_type>::callback_ptr_type> _menu_item_callbacks;

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

    [[nodiscard]] std::shared_ptr<menu_item_widget<value_type>> get_selected_menu_item() const noexcept
    {
        ttlet i = get_value_as_index();
        if (i >= 0 && i < std::ssize(_menu_item_widgets)) {
            return _menu_item_widgets[i];
        } else {
            return {};
        }
    }

    void start_selecting() noexcept
    {
        _selecting = true;
        if (auto selected_menu_item = get_selected_menu_item()) {
            this->window.update_keyboard_target(selected_menu_item, keyboard_focus_group::menu);
        }
    }

    void stop_selecting() noexcept
    {
        _selecting = false;
        window.request_redraw(aarect{_overlay_widget->local_to_window() * _overlay_widget->clipping_rectangle()});
        window.request_redraw(aarect{_local_to_window * _clipping_rectangle});
    }

    /** Populate the scroll view with menu items corresponding to the options.
     */
    void repopulate_options() noexcept
    {
        ttlet lock = std::scoped_lock(gui_system_mutex);
        auto option_list_ = *option_list;

        // If any of the options has a an icon, all of the options should show the icon.
        auto show_icon = false;
        for (ttlet & [ tag, label ] : option_list_) {
            show_icon |= label.has_icon();
        }

        _column_widget->clear();
        _menu_item_widgets.clear();
        _menu_item_callbacks.clear();
        for (ttlet & [ tag, text ] : option_list_) {
            auto menu_item = _column_widget->make_widget<menu_item_widget<value_type>>(tag, this->value);
            menu_item->set_show_check_mark(true);
            menu_item->set_show_icon(show_icon);
            menu_item->label = text;

            _menu_item_callbacks.push_back(menu_item->subscribe([this, tag] {
                this->value = tag;
                this->_selecting = false;
            }));

            _menu_item_widgets.push_back(std::move(menu_item));
        }

        _max_option_label_height = 0.0f;
        for (ttlet & [ tag, text ] : *option_list) {
            _max_option_label_height = std::max(
                _max_option_label_height,
                stencil::make_unique(alignment::middle_left, text, theme::global->labelStyle)->preferred_extent().height());
        }
    }

    void draw_outline(draw_context context) noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        context.draw_box_with_border_inside(
            rectangle(), background_color(), focus_color(), corner_shapes{theme::global->roundingRadius});
    }

    void draw_left_box(draw_context context) noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        ttlet corner_shapes = tt::corner_shapes{theme::global->roundingRadius, 0.0f, theme::global->roundingRadius, 0.0f};
        context.draw_box(translate_z(0.1f) * _left_box_rectangle, focus_color(), corner_shapes);
    }

    void draw_chevrons(draw_context context) noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());

        context.draw_glyph(_chevrons_glyph, translate_z(0.2f) * _chevrons_rectangle, label_color());
    }

    void draw_value(draw_context context) noexcept
    {
        tt_axiom(gui_system_mutex.recurse_lock_count());
        _text_stencil->draw(context, label_color(), translate_z(0.1f));
    }
};

} // namespace tt
