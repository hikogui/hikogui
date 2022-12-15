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
class checkbox_widget final : public abstract_button_widget {
public:
    using super = abstract_button_widget;
    using delegate_type = typename super::delegate_type;

    /** Construct a checkbox widget.
     *
     * @param parent The parent widget that owns this checkbox widget.
     * @param delegate The delegate to use to manage the state of the checkbox button.
     * @param attributes Different attributes used to configure the label's on the checkbox button:
     *                   a `label`, `alignment` or `semantic_text_style`. If one label is
     *                   passed it will be shown in all states. If two or three labels are passed
     *                   the labels are shown in on-state, off-state and other-state in that order.
     */
    checkbox_widget(
        widget *parent,
        std::shared_ptr<delegate_type> delegate,
        button_widget_attribute auto&&...attributes) noexcept :
        super(parent, std::move(delegate))
    {
        alignment = alignment::top_left();
        set_attributes<0>(hi_forward(attributes)...);
    }

    /** Construct a checkbox widget with a default button delegate.
     *
     * @see default_button_delegate
     * @param parent The parent widget that owns this checkbox widget.
     * @param value The value or `observer` value which represents the state of the checkbox.
     * @param attributes Different attributes used to configure the label's on the checkbox button:
     *                   a `label`, `alignment` or `semantic_text_style`. If one label is
     *                   passed it will be shown in all states. If two or three labels are passed
     *                   the labels are shown in on-state, off-state and other-state in that order.
     */
    checkbox_widget(
        widget *parent,
        different_from<std::shared_ptr<delegate_type>> auto&& value,
        button_widget_attribute auto&&...attributes) noexcept requires requires
    {
        make_default_toggle_button_delegate(hi_forward(value));
    } : checkbox_widget(parent, make_default_toggle_button_delegate(hi_forward(value)), hi_forward(attributes)...) {}

    /** Construct a checkbox widget with a default button delegate.
     *
     * @see default_button_delegate
     * @param parent The parent widget that owns this checkbox widget.
     * @param value The value or `observer` value which represents the state of the checkbox.
     * @param on_value The on-value. This value is used to determine which value yields an 'on' state.
     * @param attributes Different attributes used to configure the label's on the checkbox button:
     *                   a `label`, `alignment` or `semantic_text_style`. If one label is
     *                   passed it will be shown in all states. If two or three labels are passed
     *                   the labels are shown in on-state, off-state and other-state in that order.
     */
    template<
        different_from<std::shared_ptr<delegate_type>> Value,
        forward_of<observer<observer_decay_t<Value>>> OnValue,
        button_widget_attribute... Attributes>
    checkbox_widget(widget *parent, Value&& value, OnValue&& on_value, Attributes&&...attributes) noexcept
        requires requires
    {
        make_default_toggle_button_delegate(hi_forward(value), hi_forward(on_value));
    } :
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
     *                   a `label`, `alignment` or `semantic_text_style`. If one label is
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
        Attributes&&...attributes) noexcept requires requires
    {
        make_default_toggle_button_delegate(hi_forward(value), hi_forward(on_value), hi_forward(off_value));
    } :
        checkbox_widget(
            parent,
            make_default_toggle_button_delegate(hi_forward(value), hi_forward(on_value), hi_forward(off_value)),
            hi_forward(attributes)...)
    {
    }

    /// @privatesection
    [[nodiscard]] box_constraints update_constraints() noexcept override;
    void set_layout(widget_layout const& context) noexcept override;
    void draw(draw_context const& context) noexcept override;
    /// @endprivatesection
private:
    box_constraints _label_constraints;

    extent2i _button_size;
    aarectanglei _button_rectangle;
    glyph_ids _check_glyph;
    aarectanglei _check_glyph_rectangle;
    glyph_ids _minus_glyph;
    aarectanglei _minus_glyph_rectangle;

    void draw_check_box(draw_context const& context) noexcept;
    void draw_check_mark(draw_context const& context) noexcept;
};

}} // namespace hi::v1
