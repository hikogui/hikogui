// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/selection_widget.hpp Defines selection_widget.
 * @ingroup widgets
 */

#pragma once

#include "../GUI/module.hpp"
#include "label_widget.hpp"
#include "overlay_widget.hpp"
#include "scroll_widget.hpp"
#include "row_column_widget.hpp"
#include "menu_button_widget.hpp"
#include "selection_delegate.hpp"
#include "../observer.hpp"
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>

namespace hi { inline namespace v1 {

template<typename Context>
concept selection_widget_attribute = label_widget_attribute<Context>;

/** A graphical control element that allows the user to choose only one of a
 * predefined set of mutually exclusive options.
 *
 * @image html selection_widget.gif
 *
 * The following example creates a selection widget with three options.
 * which will monitor and modify `value` to display the options from
 * the `option_list`. At application startup, value is zero and none
 * of the options is selected:
 *
 * @snippet widgets/selection_example_impl.cpp Create selection
 *
 * @ingroup widgets
 */
template<fixed_string Name = "">
class selection_widget final : public widget {
public:
    using super = widget;
    constexpr static auto prefix = Name / "selection";

    using delegate_type = selection_delegate;

    std::shared_ptr<delegate_type> delegate;

    /** The label to show when nothing is selected.
     */
    observer<label> off_label;

    /** How the label and icon are aligned. Different layouts:
     *  - `alignment::top_left`: icon and text are inline with each other, with
     *    the icon in the top-left corner.
     *  - `alignment::top_right`: icon and text are inline with each other, with
     *    the icon in the top-right corner.
     *  - `alignment::middle_left`: icon and text are inline with each other, with
     *    the icon in the middle-left.
     *  - `alignment::middle_right`: icon and text are inline with each other, with
     *    the icon in the middle-right.
     *  - `alignment::bottom_left`: icon and text are inline with each other, with
     *    the icon in the bottom-left.
     *  - `alignment::bottom_right`: icon and text are inline with each other, with
     *    the icon in the bottom-right.
     *  - `alignment::top_center`: Larger icon above the text, both center aligned.
     *  - `alignment::bottom_center`: Larger icon below the text, both center aligned.
     *  - `alignment::middle_center`: text drawn across a large icon. Should only be
     *    used with a `pixmap` icon.
     */
    observer<alignment> alignment = hi::alignment::middle_flush();

    ~selection_widget()
    {
        hi_assert_not_null(delegate);
        delegate->deinit(*this);
    }

    /** Construct a selection widget with a delegate.
     *
     * @param parent The owner of the selection widget.
     * @param delegate The delegate which will control the selection widget.
     */
    selection_widget(widget *parent, std::shared_ptr<delegate_type> delegate) noexcept :
        super(parent), delegate(std::move(delegate))
    {
        hi_assert_not_null(this->delegate);

        _current_label_widget = std::make_unique<label_widget<prefix>>(this, alignment);
        _current_label_widget->mode = widget_mode::invisible;
        _off_label_widget = std::make_unique<label_widget<prefix / "off">>(this, off_label, alignment);

        _overlay_widget = std::make_unique<overlay_widget<prefix>>(this);
        _overlay_widget->mode = widget_mode::invisible;
        _scroll_widget = &_overlay_widget->make_widget<vertical_scroll_widget<prefix>>();
        _column_widget = &_scroll_widget->make_widget<column_widget<prefix>>();

        _off_label_cbt = this->off_label.subscribe([&](auto...) {
            ++global_counter<"selection_widget:off_label:constrain">;
            process_event({gui_event_type::window_reconstrain});
        });

        _delegate_cbt = this->delegate->subscribe([&] {
            _notification_from_delegate = true;
            ++global_counter<"selection_widget:delegate:constrain">;
            process_event({gui_event_type::window_reconstrain});
        });

        this->delegate->init(*this);
    }
    /** Construct a selection widget with a delegate.
     *
     * @param parent The owner of the selection widget.
     * @param delegate The delegate which will control the selection widget.
     * @param first_attribute First of @a attributes.
     * @param attributes Different attributes used to configure the label's on the selection box:
     *                   a `label`, `alignment`. If an label is passed
     *                   it is used as the label to show in the off-state.
     */
    selection_widget(
        widget *parent,
        std::shared_ptr<delegate_type> delegate,
        selection_widget_attribute auto&& first_attribute,
        selection_widget_attribute auto&&...attributes) noexcept :
        selection_widget(parent, std::move(delegate))
    {
        set_attributes(hi_forward(first_attribute), hi_forward(attributes)...);
    }

    /** Construct a selection widget which will monitor an option list and a
     * value.
     *
     * @param parent The owner of the selection widget.
     * @param value The value or observer value to monitor.
     * @param option_list An vector or an observer vector of pairs of keys and
     *                    labels. The keys are of the same type as the @a value.
     *                    The labels are of type `label`.
     * @param attributes Different attributes used to configure the label's on the selection box:
     *                   a `label`, `alignment`. If an label is passed
     *                   it is used as the label to show in the off-state.
     */
    template<
        different_from<std::shared_ptr<delegate_type>> Value,
        forward_of<observer<std::vector<std::pair<observer_decay_t<Value>, label>>>> OptionList,
        selection_widget_attribute... Attributes>
    selection_widget(widget *parent, Value&& value, OptionList&& option_list, Attributes&&...attributes) noexcept
        requires requires { make_default_selection_delegate(hi_forward(value), hi_forward(option_list)); }
        :
        selection_widget(
            parent,
            make_default_selection_delegate(hi_forward(value), hi_forward(option_list)),
            hi_forward(attributes)...)
    {
    }

    /** Construct a selection widget which will monitor an option list and a
     * value.
     *
     * @param parent The owner of the selection widget.
     * @param value The value or observer value to monitor.
     * @param option_list An vector or an observer vector of pairs of keys and
     *                    labels. The keys are of the same type as the @a value.
     *                    The labels are of type `label`.
     * @param off_value An optional off-value. This value is used to determine which
     *             value yields an 'off' state.
     * @param attributes Different attributes used to configure the label's on the selection box:
     *                   a `label`, `alignment`. If an label is passed
     *                   it is used as the label to show in the off-state.
     */
    template<
        different_from<std::shared_ptr<delegate_type>> Value,
        forward_of<observer<std::vector<std::pair<observer_decay_t<Value>, label>>>> OptionList,
        forward_of<observer<observer_decay_t<Value>>> OffValue,
        selection_widget_attribute... Attributes>
    selection_widget(
        widget *parent,
        Value&& value,
        OptionList&& option_list,
        OffValue&& off_value,
        Attributes&&...attributes) noexcept
        requires requires { make_default_selection_delegate(hi_forward(value), hi_forward(option_list), hi_forward(off_value)); }
        :
        selection_widget(
            parent,
            make_default_selection_delegate(hi_forward(value), hi_forward(option_list), hi_forward(off_value)),
            hi_forward(attributes)...)
    {
    }

    /// @privatesection
    [[nodiscard]] generator<widget const&> children(bool include_invisible) const noexcept override
    {
        co_yield *_overlay_widget;
        co_yield *_current_label_widget;
        co_yield *_off_label_widget;
    }

    [[nodiscard]] box_constraints update_constraints() noexcept override
    {
        hi_assert_not_null(_off_label_widget);
        hi_assert_not_null(_current_label_widget);
        hi_assert_not_null(_overlay_widget);

        if (_notification_from_delegate.exchange(false)) {
            repopulate_options();
        }

        _off_label_constraints = _off_label_widget->update_constraints();
        _current_label_constraints = _current_label_widget->update_constraints();
        _overlay_constraints = _overlay_widget->update_constraints();

        // The theme's width is used for the little chevron/icon element.
        // The labels' margins are included in the size of the widget.
        auto r = max(_off_label_constraints, _current_label_constraints);
        r.minimum.width() += theme<prefix>.width(this) + r.margins.left() + r.margins.right();
        r.minimum.height() += r.margins.bottom() + r.margins.top();
        r.preferred.width() += theme<prefix>.width(this) + r.margins.left() + r.margins.right();
        r.preferred.height() += r.margins.bottom() + r.margins.top();
        r.maximum.width() += theme<prefix>.width(this) + r.margins.left() + r.margins.right();
        r.maximum.height() += r.margins.bottom() + r.margins.top();

        // Make it so that the scroll widget can scroll vertically.
        // Set it to something small, this just fixes an issue when there are no menu items.
        _scroll_widget->minimum.copy()->height() = 10;

        // Increase the width to match the popup's width.
        inplace_max(r.minimum.width(), _overlay_constraints.minimum.width() + theme<prefix>.width(this));
        inplace_max(r.preferred.width(), _overlay_constraints.preferred.width() + theme<prefix>.width(this));
        inplace_max(r.maximum.width(), _overlay_constraints.maximum.width() + theme<prefix>.width(this));

        r.alignment = resolve(*alignment, os_settings::left_to_right());
        r.margins = theme<prefix>.margin(this);
        hi_axiom(r.holds_invariant());
        return r;
    }

    void set_layout(widget_layout const& context) noexcept override
    {
        hilet label_margins = max(_off_label_constraints.margins, _current_label_constraints.margins);
        hilet chevron_box_width = theme<prefix>.width(this);
        hilet cap_height = theme<prefix>.cap_height(this);

        if (compare_store(layout, context)) {
            if (os_settings::left_to_right()) {
                _chevron_box_rectangle = aarectanglei{0, 0, chevron_box_width, context.height()};

                // The label is located to the right of the selection box icon.
                hilet option_rectangle = aarectanglei{
                    chevron_box_width + label_margins.left(),
                    label_margins.bottom(),
                    context.width() - chevron_box_width - label_margins.left() - label_margins.right(),
                    context.height() - label_margins.bottom() - label_margins.top()};

                _off_label_shape = box_shape{_off_label_constraints, option_rectangle, cap_height};
                _current_label_shape = box_shape{_current_label_constraints, option_rectangle, cap_height};

            } else {
                _chevron_box_rectangle =
                    aarectanglei{context.width() - chevron_box_width, 0, chevron_box_width, context.height()};

                // The label is located to the left of the selection box icon.
                hilet option_rectangle = aarectanglei{
                    label_margins.left(),
                    label_margins.bottom(),
                    context.width() - _chevron_box_rectangle.width() - label_margins.left() - label_margins.right(),
                    context.height() - label_margins.bottom() - label_margins.top()};

                _off_label_shape = box_shape{_off_label_constraints, option_rectangle, cap_height};
                _current_label_shape = box_shape{_current_label_constraints, option_rectangle, cap_height};
            }

            _chevron_glyph = find_glyph(elusive_icon::ChevronUp);
            hilet chevron_glyph_bbox =
                narrow_cast<aarectanglei>(_chevron_glyph.get_bounding_rectangle() * theme<prefix>.line_height(this));
            _chevron_rectangle = align(_chevron_box_rectangle, chevron_glyph_bbox, alignment::middle_center());
        }

        // The overlay itself will make sure the overlay fits the window, so we give the preferred size and position
        // from the point of view of the selection widget.
        // The overlay should start on the same left edge as the selection box and the same width.
        // The height of the overlay should be the maximum height, which will show all the options.
        hilet overlay_width = std::clamp(
            context.width() - chevron_box_width, _overlay_constraints.minimum.width(), _overlay_constraints.maximum.width());
        hilet overlay_height = _overlay_constraints.preferred.height();
        hilet overlay_x = os_settings::left_to_right() ? chevron_box_width : context.width() - chevron_box_width - overlay_width;
        hilet overlay_y = (context.height() - overlay_height) / 2;
        hilet overlay_rectangle_request = aarectanglei{overlay_x, overlay_y, overlay_width, overlay_height};
        hilet overlay_rectangle = make_overlay_rectangle(overlay_rectangle_request);
        _overlay_shape = box_shape{_overlay_constraints, overlay_rectangle, cap_height};
        _overlay_widget->set_layout(context.transform(_overlay_shape, 20.0f));

        _off_label_widget->set_layout(context.transform(_off_label_shape));
        _current_label_widget->set_layout(context.transform(_current_label_shape));
    }

    void draw(widget_draw_context& context) noexcept override
    {
        if (*mode > widget_mode::invisible) {
            if (overlaps(context, layout)) {
                draw_outline(context);
                draw_chevron_box(context);
                draw_chevron(context);

                _off_label_widget->draw(context);
                _current_label_widget->draw(context);
            }

            // Overlay is outside of the overlap of the selection widget.
            _overlay_widget->draw(context);
        }
    }

    bool handle_event(gui_event const& event) noexcept override
    {
        switch (event.type()) {
        case gui_event_type::mouse_up:
            if (*mode >= widget_mode::partial and _has_options and layout.rectangle().contains(event.mouse().position)) {
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
            ++global_counter<"selection_widget:gui_activate:relayout">;
            process_event({gui_event_type::window_relayout});
            return true;

        case gui_event_type::gui_cancel:
            if (*mode >= widget_mode::partial and _has_options and _selecting) {
                stop_selecting();
            }
            ++global_counter<"selection_widget:gui_cancel:relayout">;
            process_event({gui_event_type::window_relayout});
            return true;

        default:;
        }

        return super::handle_event(event);
    }

    [[nodiscard]] hitbox hitbox_test(point2i position) const noexcept override
    {
        hi_axiom(loop::main().on_thread());

        if (*mode >= widget_mode::partial) {
            auto r = _overlay_widget->hitbox_test_from_parent(position);

            if (layout.contains(position)) {
                r = std::max(r, hitbox{id, layout.elevation, _has_options ? hitbox_type::button : hitbox_type::_default});
            }

            return r;
        } else {
            return {};
        }
    }

    [[nodiscard]] bool accepts_keyboard_focus(keyboard_focus_group group) const noexcept override
    {
        hi_axiom(loop::main().on_thread());
        return *mode >= widget_mode::partial and to_bool(group & hi::keyboard_focus_group::normal) and _has_options;
    }
    /// @endprivatesection
private:
    notifier<>::callback_token _delegate_cbt;
    std::atomic<bool> _notification_from_delegate = true;

    std::unique_ptr<label_widget<prefix>> _current_label_widget;
    box_constraints _current_label_constraints;
    box_shape _current_label_shape;

    std::unique_ptr<label_widget<join_path(prefix, "off")>> _off_label_widget;
    box_constraints _off_label_constraints;
    box_shape _off_label_shape;

    aarectanglei _chevron_box_rectangle;

    font_book::font_glyph_type _chevron_glyph;
    aarectanglei _chevron_rectangle;

    bool _selecting = false;
    bool _has_options = false;

    std::unique_ptr<overlay_widget<prefix>> _overlay_widget;
    box_constraints _overlay_constraints;
    box_shape _overlay_shape;

    vertical_scroll_widget<prefix> *_scroll_widget = nullptr;
    column_widget<prefix> *_column_widget = nullptr;

    decltype(off_label)::callback_token _off_label_cbt;
    std::vector<menu_button_widget<prefix> *> _menu_button_widgets;
    std::vector<notifier<>::callback_token> _menu_button_tokens;

    void set_attributes() noexcept {}
    void set_attributes(label_widget_attribute auto&& first, label_widget_attribute auto&&...rest) noexcept
    {
        if constexpr (forward_of<decltype(first), observer<hi::label>>) {
            off_label = hi_forward(first);
        } else if constexpr (forward_of<decltype(first), observer<hi::alignment>>) {
            alignment = hi_forward(first);
        } else {
            hi_static_no_default();
        }

        set_attributes(hi_forward(rest)...);
    }

    [[nodiscard]] menu_button_widget<prefix> const *get_first_menu_button() const noexcept
    {
        hi_axiom(loop::main().on_thread());

        if (ssize(_menu_button_widgets) != 0) {
            return _menu_button_widgets.front();
        } else {
            return nullptr;
        }
    }

    [[nodiscard]] menu_button_widget<prefix> const *get_selected_menu_button() const noexcept
    {
        hi_axiom(loop::main().on_thread());

        for (hilet& button : _menu_button_widgets) {
            if (button->state == widget_state::on) {
                return button;
            }
        }
        return nullptr;
    }

    void start_selecting() noexcept
    {
        hi_axiom(loop::main().on_thread());

        _selecting = true;
        _overlay_widget->mode = widget_mode::enabled;
        if (auto selected_menu_button = get_selected_menu_button()) {
            process_event(gui_event::window_set_keyboard_target(selected_menu_button->id, keyboard_focus_group::menu));

        } else if (auto first_menu_button = get_first_menu_button()) {
            process_event(gui_event::window_set_keyboard_target(first_menu_button->id, keyboard_focus_group::menu));
        }

        request_redraw();
    }

    void stop_selecting() noexcept
    {
        hi_axiom(loop::main().on_thread());
        _selecting = false;
        _overlay_widget->mode = widget_mode::invisible;
        request_redraw();
    }

    /** Populate the scroll view with menu items corresponding to the options.
     */
    void repopulate_options() noexcept
    {
        hi_axiom(loop::main().on_thread());
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
            auto menu_button = &_column_widget->make_widget<menu_button_widget<prefix>>(selected, index, label, alignment);

            _menu_button_tokens.push_back(menu_button->subscribe(
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

    void draw_outline(widget_draw_context& context) noexcept
    {
        context.draw_box(
            layout,
            layout.rectangle(),
            theme<prefix>.background_color(this),
            theme<prefix>.border_color(this),
            theme<prefix>.border_width(this),
            border_side::inside,
            theme<prefix>.border_radius(this));
    }

    void draw_chevron_box(widget_draw_context& context) noexcept
    {
        auto border_radius = theme<prefix>.border_radius(this);

        if (os_settings::left_to_right()) {
            border_radius.right_bottom() = 0;
            border_radius.right_top() = 0;
        } else {
            border_radius.left_bottom() = 0;
            border_radius.left_top() = 0;
        }

        context.draw_box(
            layout,
            translate_z(0.1f) * narrow_cast<aarectangle>(_chevron_box_rectangle),
            theme<prefix>.border_color(this),
            border_radius);
    }

    void draw_chevron(widget_draw_context& context) noexcept
    {
        context.draw_glyph(
            layout,
            translate_z(0.2f) * narrow_cast<aarectangle>(_chevron_rectangle),
            *_chevron_glyph.font,
            _chevron_glyph.glyph,
            theme<prefix>.fill_color(this));
    }
};

}} // namespace hi::v1
