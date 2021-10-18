// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "selection_widget.hpp"
#include "../text/font_book.hpp"
#include "../GUI/gui_system.hpp"
#include "../GFX/pipeline_SDF_device_shared.hpp"

namespace tt {

selection_widget::~selection_widget()
{
    if (auto delegate = _delegate.lock()) {
        delegate->deinit(*this);
    }
}

selection_widget::selection_widget(gui_window &window, widget *parent, weak_or_unique_ptr<delegate_type> delegate) noexcept :
    super(window, parent), _delegate(std::move(delegate))
{
    _current_label_widget = std::make_unique<label_widget>(window, this, l10n("<current>"));
    _current_label_widget->visible = false;
    _current_label_widget->alignment = alignment::middle_left;
    _unknown_label_widget = std::make_unique<label_widget>(window, this, unknown_label);
    _unknown_label_widget->alignment = alignment::middle_left;
    _unknown_label_widget->text_style = theme_text_style::placeholder;

    _overlay_widget = std::make_unique<overlay_widget>(window, this);
    _overlay_widget->visible = false;
    _scroll_widget = &_overlay_widget->make_widget<vertical_scroll_widget<>>();
    _column_widget = &_scroll_widget->make_widget<column_widget>();

    _unknown_label_callback = this->unknown_label.subscribe([this] {
        request_reconstrain();
    });

    if (auto d = _delegate.lock()) {
        _delegate_callback = d->subscribe(*this, [this] {
            this->window.gui.run([this] {
                repopulate_options();
                request_reconstrain();
            });
        });

        (*_delegate_callback)();

        d->init(*this);
    }
}

selection_widget::selection_widget(gui_window &window, widget *parent, std::weak_ptr<delegate_type> delegate) noexcept :
    selection_widget(window, parent, weak_or_unique_ptr<delegate_type>{std::move(delegate)})
{
}

[[nodiscard]] bool selection_widget::constrain(utc_nanoseconds display_time_point, bool need_reconstrain) noexcept
{
    tt_axiom(is_gui_thread());

    auto updated = super::constrain(display_time_point, need_reconstrain);
    if (updated) {
        ttlet extra_size = extent2{theme().size + theme().margin * 2.0f, theme().margin * 2.0f};

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

void selection_widget::layout(extent2 new_size, utc_nanoseconds display_time_point, bool need_layout) noexcept
{
    tt_axiom(is_gui_thread());

    need_layout |= _relayout.exchange(false);
    if (need_layout) {
        // The overlay itself will make sure the overlay fits the window, so we give the preferred size and position
        // from the point of view of the selection widget.

        // The overlay should start on the same left edge as the selection box and the same width.
        // The height of the overlay should be the maximum height, which will show all the options.

        ttlet overlay_width = std::clamp(
            rectangle().width() - theme().size, _overlay_widget->minimum_size().width(), _overlay_widget->maximum_size().width());
        ttlet overlay_height = _overlay_widget->preferred_size().height();
        ttlet overlay_x = theme().size;
        ttlet overlay_y = std::round(_size.height() * 0.5f - overlay_height * 0.5f);
        ttlet overlay_rectangle_request = aarectangle{overlay_x, overlay_y, overlay_width, overlay_height};

        ttlet overlay_rectangle = make_overlay_rectangle(overlay_rectangle_request);
        ttlet overlay_clipping_rectangle = expand(overlay_rectangle, _overlay_widget->margin());

        if (_overlay_widget->visible) {
            _overlay_widget->set_layout_parameters_from_parent(
                overlay_rectangle, overlay_clipping_rectangle, _overlay_widget->draw_layer - draw_layer);
            _overlay_widget->layout(overlay_rectangle.size(), display_time_point, need_layout);
        }

        _left_box_rectangle = aarectangle{0.0f, 0.0f, theme().size, rectangle().height()};
        _chevrons_glyph = font_book().find_glyph(elusive_icon::ChevronUp);
        ttlet chevrons_glyph_bbox = _chevrons_glyph.get_bounding_box();
        _chevrons_rectangle = align(_left_box_rectangle, scale(chevrons_glyph_bbox, theme().icon_size), alignment::middle_center);
        _chevrons_rectangle = align(_left_box_rectangle, scale(chevrons_glyph_bbox, theme().icon_size), alignment::middle_center);

        // The unknown_label is located to the right of the selection box icon.
        _option_rectangle = aarectangle{
            _left_box_rectangle.right() + theme().margin,
            0.0f,
            rectangle().width() - _left_box_rectangle.width() - theme().margin * 2.0f,
            rectangle().height()};

        _unknown_label_widget->set_layout_parameters_from_parent(_option_rectangle);
        if (_unknown_label_widget->visible) {
            _unknown_label_widget->layout(_option_rectangle.size(), display_time_point, need_layout);
        }
        _current_label_widget->set_layout_parameters_from_parent(_option_rectangle);
        if (_current_label_widget->visible) {
            _current_label_widget->layout(_option_rectangle.size(), display_time_point, need_layout);
        }
        request_redraw();
    }
}

void selection_widget::draw(draw_context context, utc_nanoseconds display_time_point) noexcept
{
    tt_axiom(is_gui_thread());

    if (overlaps(context, this->_clipping_rectangle)) {
        draw_outline(context);
        draw_left_box(context);
        draw_chevrons(context);
    }

    super::draw(std::move(context), display_time_point);
}

bool selection_widget::handle_event(mouse_event const &event) noexcept
{
    tt_axiom(is_gui_thread());
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

bool selection_widget::handle_event(command command) noexcept
{
    tt_axiom(is_gui_thread());
    request_relayout();

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

[[nodiscard]] hitbox selection_widget::hitbox_test(point2 position) const noexcept
{
    tt_axiom(is_gui_thread());

    auto r = super::hitbox_test(position);
    if (_visible_rectangle.contains(position)) {
        r = std::max(r, hitbox{this, draw_layer, (enabled and _has_options) ? hitbox::Type::Button : hitbox::Type::Default});
    }

    return r;
}

[[nodiscard]] bool selection_widget::accepts_keyboard_focus(keyboard_focus_group group) const noexcept
{
    tt_axiom(is_gui_thread());
    return is_normal(group) and enabled and _has_options;
}

[[nodiscard]] color selection_widget::focus_color() const noexcept
{
    tt_axiom(is_gui_thread());

    if (enabled and _has_options and _selecting) {
        return theme().color(theme_color::accent);
    } else {
        return super::focus_color();
    }
}

[[nodiscard]] menu_button_widget const *selection_widget::get_first_menu_button() const noexcept
{
    tt_axiom(is_gui_thread());

    if (std::ssize(_menu_button_widgets) != 0) {
        return _menu_button_widgets.front();
    } else {
        return nullptr;
    }
}

[[nodiscard]] menu_button_widget const *selection_widget::get_selected_menu_button() const noexcept
{
    tt_axiom(is_gui_thread());

    for (ttlet &button : _menu_button_widgets) {
        if (button->state() == button_state::on) {
            return button;
        }
    }
    return nullptr;
}

void selection_widget::start_selecting() noexcept
{
    tt_axiom(is_gui_thread());

    _selecting = true;
    _overlay_widget->visible = true;
    if (auto selected_menu_button = get_selected_menu_button()) {
        this->window.update_keyboard_target(selected_menu_button, keyboard_focus_group::menu);

    } else if (auto first_menu_button = get_first_menu_button()) {
        this->window.update_keyboard_target(first_menu_button, keyboard_focus_group::menu);
    }

    request_redraw();
}

void selection_widget::stop_selecting() noexcept
{
    tt_axiom(is_gui_thread());
    _selecting = false;
    _overlay_widget->visible = false;
    request_redraw();
}

/** Populate the scroll view with menu items corresponding to the options.
 */
void selection_widget::repopulate_options() noexcept
{
    tt_axiom(is_gui_thread());
    _column_widget->clear();
    _menu_button_widgets.clear();
    _menu_button_callbacks.clear();

    auto options = std::vector<label>{};
    auto selected = -1_z;
    if (auto delegate = _delegate.lock()) {
        std::tie(options, selected) = delegate->options_and_selected(*this);
    }

    _has_options = std::size(options) > 0;

    // If any of the options has a an icon, all of the options should show the icon.
    auto show_icon = false;
    for (ttlet &label : options) {
        show_icon |= static_cast<bool>(label.icon);
    }

    decltype(selected) index = 0;
    for (auto &&label : options) {
        auto menu_button = &_column_widget->make_widget<menu_button_widget>(std::move(label), selected, index);

        _menu_button_callbacks.push_back(menu_button->subscribe([this, index] {
            window.gui.run([this, index] {
                if (auto delegate = _delegate.lock()) {
                    delegate->set_selected(*this, index);
                }
                stop_selecting();
            });
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

void selection_widget::draw_outline(draw_context context) noexcept
{
    tt_axiom(is_gui_thread());

    context.draw_box_with_border_inside(rectangle(), background_color(), focus_color(), corner_shapes{theme().rounding_radius});
}

void selection_widget::draw_left_box(draw_context context) noexcept
{
    tt_axiom(is_gui_thread());

    ttlet corner_shapes = tt::corner_shapes{theme().rounding_radius, 0.0f, theme().rounding_radius, 0.0f};
    context.draw_box(translate_z(0.1f) * _left_box_rectangle, focus_color(), corner_shapes);
}

void selection_widget::draw_chevrons(draw_context context) noexcept
{
    tt_axiom(is_gui_thread());

    context.draw_glyph(_chevrons_glyph, theme().icon_size, translate_z(0.2f) * _chevrons_rectangle, label_color());
}

} // namespace tt
