// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/toolbar_tab_button_widget.hpp Defines toolbar_tab_button_widget.
 * @ingroup widgets
 */

#pragma once

#include "abstract_button_widget.hpp"

namespace hi { inline namespace v1 {

/** A graphical control element that allows the user to choose only one of a
 * predefined set of mutually exclusive views of a `tab_widget`.
 *
 * A toolbar tab button generally controls a `tab_widget`, to show one of its
 * child widgets.
 *
 * A toolbar tab button has two different states with different visual
 * representation:
 *  - **on**: The toolbar tab button shows raised among the other tabs.
 *  - **other**: The toolbar tab button is at equal height to other tabs.
 *
 * @image html toolbar_tab_button_widget.gif
 *
 * Each time a user activates the toolbar tab button it switches its state to
 * 'on'.
 *
 * A toolbar tab button cannot itself switch state to 'other', this state may be
 * caused by external factors. The canonical example is another toolbar tab
 * button in a set, which is configured with a different `on_value`.
 *
 * In the following example we create three toolbar tab button widgets on the
 * window which observes the same `value`. Each tab button is configured with a
 * different `on_value`: 0, 1 and 2.
 *
 * @snippet widgets/tab_example_impl.cpp Create three toolbar tab buttons
 *
 * @ingroup widgets
 * @note A toolbar tab button does not directly control a `tab_widget`. Like
 *       `radio_button_widget` this is accomplished by sharing a delegate or a
 *       observer between the toolbar tab button and the tab widget.
 */
template<fixed_string Name = "">
class toolbar_tab_button_widget final : public abstract_button_widget<Name / "toolbar-tab-button"> {
public:
    using super = abstract_button_widget<Name / "toolbar-tab-button">;
    using delegate_type = typename super::delegate_type;

    /** Construct a toolbar tab button widget.
     *
     * @param parent The parent widget that owns this radio button widget.
     * @param delegate The delegate to use to manage the state of the tab button widget.
     * @param attributes Different attributes used to configure the label's on the toolbar tab button:
     *                   a `label`, `alignment` or `text_theme`. If one label is
     *                   passed it will be shown in all states. If two labels are passed
     *                   the first label is shown in on-state and the second for off-state.
     */
    toolbar_tab_button_widget(
        widget *parent,
        std::shared_ptr<delegate_type> delegate,
        button_widget_attribute auto&&...attributes) noexcept :
        super(parent, std::move(delegate))
    {
        this->alignment = alignment::top_center();
        this->set_attributes<0>(hi_forward(attributes)...);
    }

    /** Construct a toolbar tab button widget with a default button delegate.
     *
     * @param parent The parent widget that owns this toolbar tab button widget.
     * @param value The value or `observer` value which represents the state
     *              of the toolbar tab button.
     * @param on_value An optional on-value. This value is used to determine which
     *             value yields an 'on' state.
     * @param attributes Different attributes used to configure the label's on the toolbar tab button:
     *                   a `label`, `alignment` or `text_theme`. If one label is
     *                   passed it will be shown in all states. If two labels are passed
     *                   the first label is shown in on-state and the second for off-state.
     */
    template<
        different_from<std::shared_ptr<delegate_type>> Value,
        forward_of<observer<observer_decay_t<Value>>> OnValue,
        button_widget_attribute... Attributes>
    toolbar_tab_button_widget(widget *parent, Value&& value, OnValue&& on_value, Attributes&&...attributes) noexcept
        requires requires { make_default_radio_button_delegate(hi_forward(value), hi_forward(on_value)); }
        :
        toolbar_tab_button_widget(
            parent,
            make_default_radio_button_delegate(hi_forward(value), hi_forward(on_value)),
            hi_forward(attributes)...)
    {
    }

    void request_redraw() const noexcept override
    {
        // A toolbar tab button draws a focus line across the whole toolbar
        // which is beyond it's own clipping rectangle. The parent is the toolbar
        // so it will include everything that needs to be redrawn.
        if (this->parent != nullptr) {
            this->parent->request_redraw();
        } else {
            super::request_redraw();
        }
    }

    /// @privatesection
    [[nodiscard]] box_constraints update_constraints() noexcept override
    {
        _label_constraints = super::update_constraints();

        // On left side a check mark, on right side short-cut. Around the label extra margin.
        hilet extra_size =
            extent2i{theme<super::prefix>.spacing_horizontal(this) * 2, theme<super::prefix>.spacing_vertical(this)};
        return _label_constraints + extra_size;
    }

    void set_layout(widget_layout const& context) noexcept override
    {
        if (compare_store(this->layout, context)) {
            hilet label_rectangle = aarectanglei{
                theme<super::prefix>.spacing_horizontal(this),
                0,
                context.width() - theme<super::prefix>.spacing_horizontal(this) * 2,
                context.height() - theme<super::prefix>.spacing_vertical(this)};
            this->_on_label_shape = this->_off_label_shape = this->_other_label_shape =
                box_shape{_label_constraints, label_rectangle, theme<super::prefix>.cap_height(this)};
        }
        super::set_layout(context);
    }

    void draw(widget_draw_context& context) noexcept override
    {
        if (*this->mode > widget_mode::invisible and overlaps(context, this->layout)) {
            draw_toolbar_tab_button(context);
            this->draw_button(context);
        }
    }
    [[nodiscard]] bool accepts_keyboard_focus(keyboard_focus_group group) const noexcept override
    {
        return *this->mode >= widget_mode::partial and to_bool(group & hi::keyboard_focus_group::toolbar);
    }

    [[nodiscard]] bool is_tab_button() const noexcept override
    {
        return true;
    }
    /// @endprivatesection
private:
    box_constraints _label_constraints;

    void draw_toolbar_tab_button(widget_draw_context& context) noexcept
    {
        // Draw the outline of the button across the clipping rectangle to clip the
        // bottom of the outline.
        hilet offset = theme<super::prefix>.margin_bottom(this) + theme<super::prefix>.border_width(this);
        hilet outline_rectangle = aarectanglei{0, -offset, this->layout.width(), this->layout.height() + offset};

        // The focus line will be drawn by the parent widget (toolbar_widget) at 0.5.
        hilet button_z = *this->focus ? translate_z(0.6f) : translate_z(0.0f);

        context.draw_box(
            this->layout,
            button_z * narrow_cast<aarectangle>(outline_rectangle),
            theme<super::prefix>.background_color(this),
            theme<super::prefix>.border_color(this),
            theme<super::prefix>.border_width(this),
            border_side::inside,
            theme<super::prefix>.border_radius(this));
    }
};

}} // namespace hi::v1
