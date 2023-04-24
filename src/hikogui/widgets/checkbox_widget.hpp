// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/checkbox_widget.hpp Defines checkbox_widget.
 * @ingroup widgets
 */

#pragma once

#include "abstract_button_widget.hpp"
#include "../log.hpp"

namespace hi { inline namespace v1 {

/** A GUI widget that permits the user to make a binary choice.
 * @ingroup widgets
 *
 * A checkbox is a button with three different states with different visual
 * representation:
 *  - **on**: A check-mark is shown inside the box, and the `checkbox_widget::on_label` is shown.
 *  - **off**: An empty box is shown, and the `checkbox_widget::off_label` is shown.
 *  - **other**: A dash is shown inside the box, and the `checkbox_widget::other_label` is shown.
 *
 * @image html checkbox_widget.gif
 *
 * Each time a user activates the checkbox-button it toggles between the 'on' and 'off' states.
 * If the checkbox is in the 'other' state an activation will switch it to
 * the 'off' state.
 *
 * A checkbox cannot itself switch state to 'other', this state may be
 * caused by external factors. The canonical example is a tree structure
 * of checkboxes; when child checkboxes have different values from each other
 * the parent checkbox state is set to 'other'.
 *
 * In the following example we create a checkbox widget on the window
 * which observes `value`. When the value is 1 the checkbox is 'on',
 * when the value is 2 the checkbox is 'off'.
 *
 * @snippet widgets/checkbox_example_impl.cpp Create a checkbox
 */
template<fixed_string Name = "">
class checkbox_widget final : public abstract_button_widget<Name / "checkbox"> {
public:
    using super = abstract_button_widget<Name / "checkbox">;
    using delegate_type = typename super::delegate_type;
    constexpr static auto prefix = super::prefix;

    /** Construct a checkbox widget.
     *
     * @param parent The parent widget that owns this checkbox widget.
     * @param delegate The delegate to use to manage the state of the checkbox button.
     * @param attributes Different attributes used to configure the label's on the checkbox button:
     *                   a `label`, `alignment` or `text_theme`. If one label is
     *                   passed it will be shown in all states. If two or three labels are passed
     *                   the labels are shown in on-state, off-state and other-state in that order.
     */
    checkbox_widget(
        widget *parent,
        std::shared_ptr<delegate_type> delegate,
        button_widget_attribute auto&&...attributes) noexcept :
        super(parent, std::move(delegate))
    {
        this->alignment = alignment::top_left();
        this->set_attributes<0>(hi_forward(attributes)...);
    }

    /** Construct a checkbox widget with a default button delegate.
     *
     * @see default_button_delegate
     * @param parent The parent widget that owns this checkbox widget.
     * @param value The value or `observer` value which represents the state of the checkbox.
     * @param attributes Different attributes used to configure the label's on the checkbox button:
     *                   a `label`, `alignment` or `text_theme`. If one label is
     *                   passed it will be shown in all states. If two or three labels are passed
     *                   the labels are shown in on-state, off-state and other-state in that order.
     */
    checkbox_widget(
        widget *parent,
        different_from<std::shared_ptr<delegate_type>> auto&& value,
        button_widget_attribute auto&&...attributes) noexcept
        requires requires { make_default_toggle_button_delegate(hi_forward(value)); }
        : checkbox_widget(parent, make_default_toggle_button_delegate(hi_forward(value)), hi_forward(attributes)...)
    {
    }

    /** Construct a checkbox widget with a default button delegate.
     *
     * @see default_button_delegate
     * @param parent The parent widget that owns this checkbox widget.
     * @param value The value or `observer` value which represents the state of the checkbox.
     * @param on_value The on-value. This value is used to determine which value yields an 'on' state.
     * @param attributes Different attributes used to configure the label's on the checkbox button:
     *                   a `label`, `alignment` or `text_theme`. If one label is
     *                   passed it will be shown in all states. If two or three labels are passed
     *                   the labels are shown in on-state, off-state and other-state in that order.
     */
    template<
        different_from<std::shared_ptr<delegate_type>> Value,
        forward_of<observer<observer_decay_t<Value>>> OnValue,
        button_widget_attribute... Attributes>
    checkbox_widget(widget *parent, Value&& value, OnValue&& on_value, Attributes&&...attributes) noexcept
        requires requires { make_default_toggle_button_delegate(hi_forward(value), hi_forward(on_value)); }
        :
        checkbox_widget(
            parent,
            make_default_toggle_button_delegate(hi_forward(value), hi_forward(on_value)),
            hi_forward(attributes)...)
    {
    }

    /** Construct a checkbox widget with a default button delegate.
     *
     * @see default_button_delegate
     * @param parent The parent widget that owns this checkbox widget.
     * @param value The value or `observer` value which represents the state of the checkbox.
     * @param on_value The on-value. This value is used to determine which value yields an 'on' state.
     * @param off_value The off-value. This value is used to determine which value yields an 'off' state.
     * @param attributes Different attributes used to configure the label's on the checkbox button:
     *                   a `label`, `alignment` or `text_theme`. If one label is
     *                   passed it will be shown in all states. If two or three labels are passed
     *                   the labels are shown in on-state, off-state and other-state in that order.
     */
    template<
        different_from<std::shared_ptr<delegate_type>> Value,
        forward_of<observer<observer_decay_t<Value>>> OnValue,
        forward_of<observer<observer_decay_t<Value>>> OffValue,
        button_widget_attribute... Attributes>
    checkbox_widget(
        widget *parent,
        Value&& value,
        OnValue&& on_value,
        OffValue&& off_value,
        Attributes&&...attributes) noexcept
        requires requires { make_default_toggle_button_delegate(hi_forward(value), hi_forward(on_value), hi_forward(off_value)); }
        :
        checkbox_widget(
            parent,
            make_default_toggle_button_delegate(hi_forward(value), hi_forward(on_value), hi_forward(off_value)),
            hi_forward(attributes)...)
    {
    }

    /// @privatesection
    [[nodiscard]] box_constraints update_constraints() noexcept override
    {
        _label_constraints = super::update_constraints();

        _button_size = theme<prefix>.size(this);
        hilet extra_size = extent2i{theme<prefix>.horizontal_spacing(this) + _button_size.width(), 0};

        auto constraints = max(_label_constraints + extra_size, _button_size);
        constraints.margins = theme<prefix>.margin(this);
        constraints.alignment = *this->alignment;
        return constraints;
    }

    void set_layout(widget_layout const& context) noexcept override
    {
        if (compare_store(this->layout, context)) {
            auto alignment_ = os_settings::left_to_right() ? *this->alignment : mirror(*this->alignment);

            if (alignment_ == horizontal_alignment::left or alignment_ == horizontal_alignment::right) {
                _button_rectangle = align(context.rectangle(), _button_size, alignment_);
            } else {
                hi_not_implemented();
            }

            hilet inner_margin = theme<prefix>.horizontal_spacing(this);
            hilet baseline_offset = theme<prefix>.cap_height(this);
            hilet icon_size = theme<prefix / "icon">.size(this);

            hilet label_width = context.width() - (_button_rectangle.width() + inner_margin);
            if (alignment_ == horizontal_alignment::left) {
                hilet label_left = _button_rectangle.right() + inner_margin;
                hilet label_rectangle = aarectanglei{label_left, 0, label_width, context.height()};
                this->_on_label_shape = this->_off_label_shape = this->_other_label_shape =
                    box_shape(_label_constraints, label_rectangle, baseline_offset);

            } else if (alignment_ == horizontal_alignment::right) {
                hilet label_rectangle = aarectanglei{0, 0, label_width, context.height()};
                this->_on_label_shape = this->_off_label_shape = this->_other_label_shape =
                    box_shape(_label_constraints, label_rectangle, baseline_offset);
            } else {
                hi_not_implemented();
            }

            _check_glyph = find_glyph(elusive_icon::Ok);
            hilet check_glyph_bb = narrow_cast<aarectanglei>(_check_glyph.get_bounding_rectangle() * icon_size);
            _check_glyph_rectangle = align(_button_rectangle, check_glyph_bb, alignment::middle_center());

            _minus_glyph = find_glyph(elusive_icon::Minus);
            hilet minus_glyph_bb = narrow_cast<aarectanglei>(_minus_glyph.get_bounding_rectangle() * icon_size);
            _minus_glyph_rectangle = align(_button_rectangle, minus_glyph_bb, alignment::middle_center());
        }
        super::set_layout(context);
    }

    void draw(widget_draw_context const& context) noexcept override
    {
        if (*this->mode > widget_mode::invisible and overlaps(context, this->layout)) {
            draw_check_box(context);
            draw_check_mark(context);
            this->draw_button(context);
        }
    }
    /// @endprivatesection
private:
    box_constraints _label_constraints;

    extent2i _button_size;
    aarectanglei _button_rectangle;
    font_book::font_glyph_type _check_glyph;
    aarectanglei _check_glyph_rectangle;
    font_book::font_glyph_type _minus_glyph;
    aarectanglei _minus_glyph_rectangle;

    void draw_check_box(widget_draw_context const& context) noexcept
    {
        context.draw_box(
            this->layout,
            _button_rectangle,
            theme<prefix>.background_color(this),
            theme<prefix>.border_color(this),
            theme<prefix>.border_width(this),
            border_side::inside);
    }

    void draw_check_mark(widget_draw_context const& context) noexcept
    {
        if (this->state == widget_state::on) {
            // Checkmark
            context.draw_glyph(
                this->layout,
                translate_z(0.1f) * narrow_cast<aarectangle>(_check_glyph_rectangle),
                *_check_glyph.font,
                _check_glyph.glyph,
                theme<prefix / "icon">.fill_color(this));

        } else if (this->state == widget_state::off) {
            ;

        } else {
            // Tri-state
            context.draw_glyph(
                this->layout,
                translate_z(0.1f) * narrow_cast<aarectangle>(_minus_glyph_rectangle),
                *_minus_glyph.font,
                _minus_glyph.glyph,
                theme<prefix / "icon">.fill_color(this));
        }
    }
};

}} // namespace hi::v1
