// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "selection_widget.hpp"
#include "../text/font_book.hpp"
#include "../GUI/gui_system.hpp"
#include "../GFX/pipeline_SDF_device_shared.hpp"
#include "../loop.hpp"

namespace hi::inline v1 {

selection_widget::~selection_widget()
{
    hi_assert_not_null(delegate);
    delegate->deinit(*this);
}

selection_widget::selection_widget(gui_window& window, widget *parent, std::shared_ptr<delegate_type> delegate) noexcept :
    super(window, parent), delegate(std::move(delegate))
{
    hi_assert_not_null(this->delegate);

    alignment = alignment::middle_left();

    _current_label_widget = std::make_unique<label_widget>(window, this, alignment, text_style);
    _current_label_widget->mode = widget_mode::invisible;
    _off_label_widget = std::make_unique<label_widget>(window, this, off_label, alignment, semantic_text_style::placeholder);

    _overlay_widget = std::make_unique<overlay_widget>(window, this);
    _overlay_widget->mode = widget_mode::invisible;
    _scroll_widget = &_overlay_widget->make_widget<vertical_scroll_widget<>>();
    _column_widget = &_scroll_widget->make_widget<column_widget>();

    _off_label_cbt = this->off_label.subscribe([&](auto...) {
        hi_request_reconstrain("selection_widget::_off_label_cbt()");
    });

    _delegate_cbt = this->delegate->subscribe([&] {
        _notification_from_delegate = true;
        hi_request_reconstrain("selection_widget::_delegate_cbt()");
    });

    this->delegate->init(*this);
}

[[nodiscard]] generator<widget *> selection_widget::children() const noexcept
{
    co_yield _overlay_widget.get();
    co_yield _current_label_widget.get();
    co_yield _off_label_widget.get();
}

widget_constraints const& selection_widget::set_constraints() noexcept
{
    _layout = {};

    if (_notification_from_delegate.exchange(false)) {
        repopulate_options();
    }

    hilet extra_size = extent2{theme().size + theme().margin * 2.0f, theme().margin * 2.0f};

    _constraints = max(_off_label_widget->set_constraints() + extra_size, _current_label_widget->set_constraints() + extra_size);

    hilet overlay_constraints = _overlay_widget->set_constraints();
    for (hilet& child : _menu_button_widgets) {
        // extra_size is already implied in the menu button widgets.
        _constraints = max(_constraints, child->constraints());
    }

    _constraints.minimum.width() =
        std::max(_constraints.minimum.width(), overlay_constraints.minimum.width() + extra_size.width());
    _constraints.preferred.width() =
        std::max(_constraints.preferred.width(), overlay_constraints.preferred.width() + extra_size.width());
    _constraints.maximum.width() =
        std::max(_constraints.maximum.width(), overlay_constraints.maximum.width() + extra_size.width());
    _constraints.margins = theme().margin;
    _constraints.baseline = widget_baseline{0.9f, vertical_alignment::middle, theme().cap_height};
    hi_axiom(_constraints.holds_invariant());
    return _constraints;
}

void selection_widget::set_layout(widget_layout const& layout) noexcept
{
    if (compare_store(_layout, layout)) {
        if (layout.left_to_right()) {
            _left_box_rectangle = aarectangle{0.0f, 0.0f, theme().size, layout.height()};

            // The unknown_label is located to the right of the selection box icon.
            _option_rectangle = aarectangle{
                _left_box_rectangle.right() + theme().margin,
                0.0f,
                layout.width() - _left_box_rectangle.width() - theme().margin * 2.0f,
                layout.height()};

        } else {
            _left_box_rectangle = aarectangle{layout.width() - theme().size, 0.0f, theme().size, layout.height()};

            // The unknown_label is located to the left of the selection box icon.
            _option_rectangle = aarectangle{
                theme().margin,
                0.0f,
                layout.width() - _left_box_rectangle.width() - theme().margin * 2.0f,
                layout.height()};
        }

        _chevrons_glyph = font_book().find_glyph(elusive_icon::ChevronUp);
        hilet chevrons_glyph_bbox = _chevrons_glyph.get_bounding_box();
        _chevrons_rectangle = align(_left_box_rectangle, chevrons_glyph_bbox * theme().icon_size, alignment::middle_center());
    }

    // The overlay itself will make sure the overlay fits the window, so we give the preferred size and position
    // from the point of view of the selection widget.
    // The overlay should start on the same left edge as the selection box and the same width.
    // The height of the overlay should be the maximum height, which will show all the options.
    hilet overlay_width = std::clamp(
        layout.width() - theme().size,
        _overlay_widget->constraints().minimum.width(),
        _overlay_widget->constraints().maximum.width());
    hilet overlay_height = _overlay_widget->constraints().preferred.height();
    hilet overlay_x = layout.left_to_right() ? theme().size : layout.width() - theme().size - overlay_width;
    hilet overlay_y = std::round(layout.height() * 0.5f - overlay_height * 0.5f);
    hilet overlay_rectangle_request = aarectangle{overlay_x, overlay_y, overlay_width, overlay_height};
    _overlay_rectangle = make_overlay_rectangle(overlay_rectangle_request);
    _overlay_widget->set_layout(layout.transform(_overlay_rectangle, 20.0f));

    _off_label_widget->set_layout(layout.transform(_option_rectangle));
    _current_label_widget->set_layout(layout.transform(_option_rectangle));
}

void selection_widget::draw(draw_context const& context) noexcept
{
    if (*mode > widget_mode::invisible) {
        if (overlaps(context, layout())) {
            draw_outline(context);
            draw_left_box(context);
            draw_chevrons(context);

            _off_label_widget->draw(context);
            _current_label_widget->draw(context);
        }

        // Overlay is outside of the overlap of the selection widget.
        _overlay_widget->draw(context);
    }
}

bool selection_widget::handle_event(gui_event const& event) noexcept
{
    switch (event.type()) {
    case gui_event_type::mouse_up:
        if (*mode >= widget_mode::partial and _has_options and layout().rectangle().contains(event.mouse().position)) {
            return handle_event(gui_event_type::gui_activate);
        }
        return true;

    case gui_event_type::gui_activate_next:
        // Handle gui_active_next so that the next widget will NOT get keyboard focus.
        // The previously selected item needs the get keyboard focus instead.
    case gui_event_type::gui_activate:
        if (*mode >= widget_mode::partial and _has_options and not _selecting) {
            start_selecting();
        } else {
            stop_selecting();
        }
        request_relayout();
        return true;

    case gui_event_type::gui_cancel:
        if (*mode >= widget_mode::partial and _has_options and _selecting) {
            stop_selecting();
        }
        request_relayout();
        return true;

    default:;
    }

    return super::handle_event(event);
}

[[nodiscard]] hitbox selection_widget::hitbox_test(point3 position) const noexcept
{
    hi_axiom(is_gui_thread());

    if (*mode >= widget_mode::partial) {
        auto r = _overlay_widget->hitbox_test_from_parent(position);

        if (layout().contains(position)) {
            r = std::max(r, hitbox{this, position, _has_options ? hitbox::Type::Button : hitbox::Type::Default});
        }

        return r;
    } else {
        return {};
    }
}

[[nodiscard]] bool selection_widget::accepts_keyboard_focus(keyboard_focus_group group) const noexcept
{
    hi_axiom(is_gui_thread());
    return *mode >= widget_mode::partial and any(group & hi::keyboard_focus_group::normal) and _has_options;
}

[[nodiscard]] color selection_widget::focus_color() const noexcept
{
    hi_axiom(is_gui_thread());

    if (*mode >= widget_mode::partial and _has_options and _selecting) {
        return theme().color(semantic_color::accent);
    } else {
        return super::focus_color();
    }
}

[[nodiscard]] menu_button_widget const *selection_widget::get_first_menu_button() const noexcept
{
    hi_axiom(is_gui_thread());

    if (ssize(_menu_button_widgets) != 0) {
        return _menu_button_widgets.front();
    } else {
        return nullptr;
    }
}

[[nodiscard]] menu_button_widget const *selection_widget::get_selected_menu_button() const noexcept
{
    hi_axiom(is_gui_thread());

    for (hilet& button : _menu_button_widgets) {
        if (button->state() == button_state::on) {
            return button;
        }
    }
    return nullptr;
}

void selection_widget::start_selecting() noexcept
{
    hi_axiom(is_gui_thread());

    _selecting = true;
    _overlay_widget->mode = widget_mode::enabled;
    if (auto selected_menu_button = get_selected_menu_button()) {
        this->window.update_keyboard_target(selected_menu_button, keyboard_focus_group::menu);

    } else if (auto first_menu_button = get_first_menu_button()) {
        this->window.update_keyboard_target(first_menu_button, keyboard_focus_group::menu);
    }

    request_redraw();
}

void selection_widget::stop_selecting() noexcept
{
    hi_axiom(is_gui_thread());
    _selecting = false;
    _overlay_widget->mode = widget_mode::invisible;
    request_redraw();
}

/** Populate the scroll view with menu items corresponding to the options.
 */
void selection_widget::repopulate_options() noexcept
{
    hi_axiom(is_gui_thread());
    hi_assert_not_null(delegate);

    _column_widget->clear();
    _menu_button_widgets.clear();
    _menu_button_tokens.clear();

    auto [options, selected] = delegate->options_and_selected(*this);

    _has_options = size(options) > 0;

    // If any of the options has a an icon, all of the options should show the icon.
    auto show_icon = false;
    for (hilet& label : options) {
        show_icon |= to_bool(label.icon);
    }

    decltype(selected) index = 0;
    for (hilet& label : options) {
        auto menu_button = &_column_widget->make_widget<menu_button_widget>(selected, index, label, alignment, text_style);

        _menu_button_tokens.push_back(menu_button->pressed.subscribe(
            [this, index] {
                hi_assert_not_null(delegate);
                delegate->set_selected(*this, index);
                stop_selecting();
            },
            callback_flags::main));

        _menu_button_widgets.push_back(menu_button);

        ++index;
    }

    if (selected == -1) {
        _off_label_widget->mode = widget_mode::display;
        _current_label_widget->mode = widget_mode::invisible;

    } else {
        _off_label_widget->mode = widget_mode::invisible;
        _current_label_widget->label = options[selected];
        _current_label_widget->mode = widget_mode::display;
    }
}

void selection_widget::draw_outline(draw_context const& context) noexcept
{
    context.draw_box(
        layout(),
        layout().rectangle(),
        background_color(),
        focus_color(),
        theme().border_width,
        border_side::inside,
        corner_radii{theme().rounding_radius});
}

void selection_widget::draw_left_box(draw_context const& context) noexcept
{
    hilet corner_radii = hi::corner_radii{theme().rounding_radius, 0.0f, theme().rounding_radius, 0.0f};
    context.draw_box(layout(), translate_z(0.1f) * _left_box_rectangle, focus_color(), corner_radii);
}

void selection_widget::draw_chevrons(draw_context const& context) noexcept
{
    context.draw_glyph(layout(), translate_z(0.2f) * _chevrons_rectangle, label_color(), _chevrons_glyph);
}

} // namespace hi::inline v1
