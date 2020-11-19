// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "widget.hpp"
#include "WindowWidget.hpp"
#include "overlay_view_widget.hpp"
#include "ScrollViewWidget.hpp"
#include "row_column_layout_widget.hpp"
#include "menu_item_widget.hpp"
#include "../stencils/label_stencil.hpp"
#include "../GUI/draw_context.hpp"
#include "../text/FontBook.hpp"
#include "../text/ElusiveIcons.hpp"
#include "../observable.hpp"
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>

namespace tt {

template<typename T>
class selection_widget final : public widget {
public:
    using super = widget;
    using value_type = T;
    using option_list_type = std::vector<std::pair<value_type, label>>;

    observable<label> unknown_label;
    observable<value_type> value;
    observable<option_list_type> option_list;

    template<typename Value = value_type, typename OptionList = option_list_type, typename UnknownLabel = label>
    selection_widget(
        gui_window &window,
        std::shared_ptr<widget> parent,
        Value &&value = value_type{},
        OptionList &&option_list = option_list_type{},
        UnknownLabel &&unknown_label = label{l10n("<unknown>")}) noexcept :
        widget(window, parent),
        value(std::forward<Value>(value)),
        option_list(std::forward<OptionList>(option_list)),
        unknown_label(std::forward<UnknownLabel>(unknown_label))
    {
    }

    ~selection_widget() {}

    void initialize() noexcept override
    {
        _overlay_widget = std::make_shared<overlay_view_widget>(window, shared_from_this());
        _overlay_widget->initialize();

        _scroll_widget = _overlay_widget->make_widget<VerticalScrollViewWidget<>>();
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
        tt_assume(gui_system_mutex.recurse_lock_count());

        auto updated = super::update_constraints(display_time_point, need_reconstrain);
        updated |= _overlay_widget->update_constraints(display_time_point, need_reconstrain);

        if (updated) {
            ttlet index = get_value_as_index();
            if (index == -1) {
                _text_stencil = stencil::make_unique(alignment::middle_left, *unknown_label, theme->placeholderLabelStyle);
                _text_stencil_color = theme->placeholderLabelStyle.color;
            } else {
                _text_stencil = stencil::make_unique(alignment::middle_left, (*option_list)[index].second, theme->labelStyle);
                _text_stencil_color = theme->labelStyle.color;
            }

            // Calculate the size of the widget based on the largest height of a label and the width of the overlay.
            ttlet unknown_label_size =
                stencil::make_unique(alignment::middle_left, *unknown_label, theme->placeholderLabelStyle)->preferred_extent();

            ttlet overlay_width = _overlay_widget->preferred_size().minimum().width();
            ttlet option_width = std::max(overlay_width, unknown_label_size.width() + Theme::margin * 2.0f);
            ttlet option_height = std::max(unknown_label_size.height(), _max_option_label_height) + Theme::margin * 2.0f;
            ttlet chevron_width = Theme::smallSize;

            _preferred_size = interval_vec2::make_minimum(vec{chevron_width + option_width, option_height});
            _preferred_base_line = relative_base_line{vertical_alignment::middle, 0.0f, 200.0f};
            return true;

        } else {
            return false;
        }
    }

    [[nodiscard]] bool update_layout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept override
    {
        tt_assume(gui_system_mutex.recurse_lock_count());

        need_layout |= std::exchange(_request_relayout, false);

        if (_selecting) {
            if (need_layout) {
                // The overlay itself will make sure the overlay fits the window, so we give the preferred size and position
                // from the point of view of the selection widget.

                // The overlay should start on the same left edge as the selection box and the same width.
                // The height of the overlay should be the maximum height, which will show all the options.

                ttlet overlay_width = clamp(rectangle().width() - Theme::smallSize, _overlay_widget->preferred_size().width());
                ttlet overlay_height = _overlay_widget->preferred_size().maximum().height();
                ttlet overlay_x = _window_rectangle.x() + Theme::smallSize;
                ttlet overlay_y = std::round(_window_rectangle.middle() - overlay_height * 0.5f);
                ttlet overlay_rectangle = aarect{overlay_x, overlay_y, overlay_width, overlay_height};

                _overlay_widget->set_layout_parameters(overlay_rectangle, overlay_rectangle);
            }
            need_layout |= _overlay_widget->update_layout(display_time_point, need_layout);
        }

        if (need_layout) {
            _left_box_rectangle = aarect{0.0f, 0.0f, Theme::smallSize, rectangle().height()};
            _chevrons_glyph = to_FontGlyphIDs(ElusiveIcon::ChevronUp);
            ttlet chevrons_glyph_bbox = PipelineSDF::DeviceShared::getBoundingBox(_chevrons_glyph);
            _chevrons_rectangle =
                align(_left_box_rectangle, scale(chevrons_glyph_bbox, Theme::small_icon_size), alignment::middle_center);
            _chevrons_rectangle =
                align(_left_box_rectangle, scale(chevrons_glyph_bbox, Theme::small_icon_size), alignment::middle_center);

            // The unknown_label is located to the right of the selection box icon.
            _option_rectangle = aarect{
                _left_box_rectangle.right() + Theme::margin,
                0.0f,
                rectangle().width() - _left_box_rectangle.width() - Theme::margin * 2.0f,
                rectangle().height()};

            _text_stencil->set_layout_parameters(_option_rectangle, base_line());
        }
        return widget::update_layout(display_time_point, need_layout);
    }

    void draw(draw_context context, hires_utc_clock::time_point display_time_point) noexcept override
    {
        tt_assume(gui_system_mutex.recurse_lock_count());

        draw_outline(context);
        draw_left_box(context);
        draw_chevrons(context);
        draw_value(context);

        if (_selecting) {
            _overlay_widget->draw(_overlay_widget->make_draw_context(context), display_time_point);
        }

        widget::draw(std::move(context), display_time_point);
    }

    bool handle_mouse_event(MouseEvent const &event) noexcept override
    {
        ttlet lock = std::scoped_lock(gui_system_mutex);
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
        ttlet lock = std::scoped_lock(gui_system_mutex);
        auto handled = widget::handle_command(command);

        if (*enabled) {
            switch (command) {
            using enum tt::command;
            case gui_activate:
                handled = true;
                if (!_selecting) {
                    start_selecting();
                } else {
                    stop_selecting();
                }
                break;
            case gui_escape:
                if (_selecting) {
                    handled = true;
                    stop_selecting();
                }
                break;
            default:;
            }
        }

        _request_relayout = true;
        return handled;
    }

    bool handle_command_recursive(command command, std::vector<std::shared_ptr<widget>> const &reject_list) noexcept override
    {
        tt_assume(gui_system_mutex.recurse_lock_count());
        tt_assume(_overlay_widget);

        auto handled = false;
        handled |= _overlay_widget->handle_command_recursive(command, reject_list); 
        handled |= super::handle_command_recursive(command, reject_list);
        return handled;
    }

    [[nodiscard]] HitBox hitbox_test(vec window_position) const noexcept override
    {
        ttlet lock = std::scoped_lock(gui_system_mutex);
        ttlet position = _from_window_transform * window_position;

        auto r = HitBox{};

        if (_selecting) {
            r = std::max(r, _overlay_widget->hitbox_test(window_position));
        }

        if (_window_clipping_rectangle.contains(window_position)) {
            r = std::max(r, HitBox{weak_from_this(), _draw_layer, *enabled ? HitBox::Type::Button : HitBox::Type::Default});
        }

        return r;
    }

    [[nodiscard]] bool accepts_focus() const noexcept override
    {
        tt_assume(gui_system_mutex.recurse_lock_count());
        return *enabled;
    }

    std::shared_ptr<widget>
    next_keyboard_widget(std::shared_ptr<widget> const &current_keyboard_widget, bool reverse) const noexcept
    {
        ttlet lock = std::scoped_lock(gui_system_mutex);

        if (_selecting) {
            tt_assume(_overlay_widget);
            if (current_keyboard_widget == shared_from_this()) {
                return _overlay_widget->next_keyboard_widget({}, reverse);
            } else {
                return _overlay_widget->next_keyboard_widget(current_keyboard_widget, reverse);
            }

        } else {
            return super::next_keyboard_widget(current_keyboard_widget, reverse);
        }
    }

private:
    typename decltype(unknown_label)::callback_ptr_type _unknown_label_callback;
    typename decltype(value)::callback_ptr_type _value_callback;
    typename decltype(option_list)::callback_ptr_type _option_list_callback;

    std::unique_ptr<label_stencil> _text_stencil;
    vec _text_stencil_color;

    float _max_option_label_height;

    aarect _option_rectangle;
    aarect _left_box_rectangle;

    FontGlyphIDs _chevrons_glyph;
    aarect _chevrons_rectangle;

    bool _selecting = false;
    std::shared_ptr<overlay_view_widget> _overlay_widget;
    std::shared_ptr<VerticalScrollViewWidget<>> _scroll_widget;
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
            this->window.update_keyboard_target(selected_menu_item);
        }
    }

    void stop_selecting() noexcept
    {
        _selecting = false;
    }


    /** Populate the scroll view with menu items corresponding to the options.
     */
    void repopulate_options() noexcept
    {
        ttlet lock = std::scoped_lock(gui_system_mutex);
        auto option_list_ = *option_list;

        // If any of the options has a an icon, all of the options should show the icon.
        auto show_icon = false;
        for (ttlet &[tag, label] : option_list_) {
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
                stencil::make_unique(alignment::middle_left, text, theme->labelStyle)->preferred_extent().height());
        }        
    }

    void draw_outline(draw_context context) noexcept
    {
        tt_assume(gui_system_mutex.recurse_lock_count());

        context.corner_shapes = Theme::roundingRadius;
        context.draw_box_with_border_inside(rectangle());
    }

    void draw_left_box(draw_context context) noexcept
    {
        tt_assume(gui_system_mutex.recurse_lock_count());

        context.transform = mat::T{0.0, 0.0, 0.1f} * context.transform;
        context.fill_color = context.color;
        context.corner_shapes = vec{Theme::roundingRadius, 0.0f, Theme::roundingRadius, 0.0f};
        context.draw_box_with_border_inside(_left_box_rectangle);
    }

    void draw_chevrons(draw_context context) noexcept
    {
        tt_assume(gui_system_mutex.recurse_lock_count());

        context.transform = mat::T{0.0, 0.0, 0.2f} * context.transform;
        context.color = *enabled ? theme->foregroundColor : context.fill_color;
        context.draw_glyph(_chevrons_glyph, _chevrons_rectangle);
    }

    void draw_value(draw_context context) noexcept
    {
        tt_assume(gui_system_mutex.recurse_lock_count());

        context.transform = mat::T{0.0, 0.0, 0.1f} * context.transform;
        context.color = *enabled ? _text_stencil_color : context.color;
        _text_stencil->draw(context, true);
    }
};

} // namespace tt
