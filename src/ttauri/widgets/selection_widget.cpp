// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "selection_widget.hpp"
#include "../text/font_book.hpp"
#include "../GUI/gui_system.hpp"
#include "../GFX/pipeline_SDF_device_shared.hpp"

namespace tt::inline v1 {

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
    _current_label_widget->alignment = alignment::middle_left();
    _unknown_label_widget = std::make_unique<label_widget>(window, this, unknown_label);
    _unknown_label_widget->alignment = alignment::middle_left();
    _unknown_label_widget->text_style = theme_text_style::placeholder;

    _overlay_widget = std::make_unique<overlay_widget>(window, this);
    _overlay_widget->visible = false;
    _scroll_widget = &_overlay_widget->make_widget<vertical_scroll_widget<>>();
    _column_widget = &_scroll_widget->make_widget<column_widget>();

    this->unknown_label.subscribe(_reconstrain_callback);

    if (auto d = _delegate.lock()) {
        _delegate_callback = d->subscribe(*this, [this] {
            this->window.gui.run([this] {
                repopulate_options();
                this->request_reconstrain();
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

widget_constraints const &selection_widget::set_constraints() noexcept
{
    _layout = {};

    ttlet extra_size = extent2{theme().size + theme().margin * 2.0f, theme().margin * 2.0f};

    _constraints =
        max(_unknown_label_widget->set_constraints() + extra_size, _current_label_widget->set_constraints() + extra_size);

    ttlet overlay_constraints = _overlay_widget->set_constraints();
    for (ttlet &child : _menu_button_widgets) {
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

    tt_axiom(_constraints.holds_invariant());
    return _constraints;
}

void selection_widget::set_layout(widget_layout const &layout) noexcept
{
    if (compare_store(_layout, layout)) {
        _left_box_rectangle = aarectangle{0.0f, 0.0f, theme().size, layout.height()};
        _chevrons_glyph = font_book().find_glyph(elusive_icon::ChevronUp);
        ttlet chevrons_glyph_bbox = _chevrons_glyph.get_bounding_box();
        _chevrons_rectangle = align(_left_box_rectangle, chevrons_glyph_bbox * theme().icon_size, alignment::middle_center());

        // The unknown_label is located to the right of the selection box icon.
        _option_rectangle = aarectangle{
            _left_box_rectangle.right() + theme().margin,
            0.0f,
            layout.width() - _left_box_rectangle.width() - theme().margin * 2.0f,
            layout.height()};
    }

    // The overlay itself will make sure the overlay fits the window, so we give the preferred size and position
    // from the point of view of the selection widget.
    // The overlay should start on the same left edge as the selection box and the same width.
    // The height of the overlay should be the maximum height, which will show all the options.
    ttlet overlay_width = std::clamp(
        layout.width() - theme().size,
        _overlay_widget->constraints().minimum.width(),
        _overlay_widget->constraints().maximum.width());
    ttlet overlay_height = _overlay_widget->constraints().preferred.height();
    ttlet overlay_x = theme().size;
    ttlet overlay_y = std::round(layout.height() * 0.5f - overlay_height * 0.5f);
    ttlet overlay_rectangle_request = aarectangle{overlay_x, overlay_y, overlay_width, overlay_height};
    _overlay_rectangle = make_overlay_rectangle(overlay_rectangle_request);
    _overlay_widget->set_layout(layout.transform(_overlay_rectangle, 20.0f));

    _unknown_label_widget->set_layout(layout.transform(_option_rectangle));
    _current_label_widget->set_layout(layout.transform(_option_rectangle));
}

void selection_widget::draw(draw_context const &context) noexcept
{
    if (visible) {
        if (overlaps(context, layout())) {
            draw_outline(context);
            draw_left_box(context);
            draw_chevrons(context);

            _unknown_label_widget->draw(context);
            _current_label_widget->draw(context);
        }

        // Overlay is outside of the overlap of the selection widget.
        _overlay_widget->draw(context);
    }
}

bool selection_widget::handle_event(mouse_event const &event) noexcept
{
    tt_axiom(is_gui_thread());
    auto handled = super::handle_event(event);

    if (event.cause.leftButton) {
        handled = true;
        if (enabled and _has_options) {
            if (event.type == mouse_event::Type::ButtonUp && layout().rectangle().contains(event.position)) {
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

        case gui_cancel:
            if (_selecting) {
                stop_selecting();
            }
            return true;

        default:;
        }
    }

    return super::handle_event(command);
}

[[nodiscard]] hitbox selection_widget::hitbox_test(point3 position) const noexcept
{
    tt_axiom(is_gui_thread());

    if (visible and enabled) {
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
    tt_axiom(is_gui_thread());
    return visible and enabled and any(group & tt::keyboard_focus_group::normal) and _has_options;
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

    if (ssize(_menu_button_widgets) != 0) {
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

    _has_options = size(options) > 0;

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

void selection_widget::draw_outline(draw_context const &context) noexcept
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

void selection_widget::draw_left_box(draw_context const &context) noexcept
{
    ttlet corner_radii = tt::corner_radii{theme().rounding_radius, 0.0f, theme().rounding_radius, 0.0f};
    context.draw_box(layout(), translate_z(0.1f) * _left_box_rectangle, focus_color(), corner_radii);
}

void selection_widget::draw_chevrons(draw_context const &context) noexcept
{
    context.draw_glyph(layout(), translate_z(0.2f) * _chevrons_rectangle, label_color(), _chevrons_glyph);
}

} // namespace tt::inline v1
