// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/toolbar_tab_button_widget.hpp Defines toolbar_tab_button_widget.
 * @ingroup widgets
 */

#pragma once

#include "widget.hpp"
#include "radio_delegate.hpp"
#include "../macros.hpp"

hi_export_module(hikogui.widgets.toolbar_tab_button_widget);

hi_export namespace hi {
inline namespace v1 {

template<typename Context>
concept toolbar_tab_button_widget_attribute = label_widget_attribute<Context>;

/** A graphical control element that allows the user to choose only one of a
 * predefined set of mutually exclusive views of a `tab_widget`.
 *
 * A toolbar tab button generally controls a `tab_widget`, to show one of its
 * child widgets.
 *
 * A toolbar tab button has two different states with different visual
 * representation:
 *  - **on**: The toolbar tab button shows raised among the other tabs.
 *  - **off**: The toolbar tab button is at equal height to other tabs.
 *
 * @image html toolbar_tab_button_widget.gif
 *
 * Each time a user activates the toolbar tab button it switches its state to
 * 'on'.
 *
 * A toolbar tab button cannot itself switch state to 'off', this state may be
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
 *       `radio_widget` this is accomplished by sharing a delegate or a
 *       observer between the toolbar tab button and the tab widget.
 */
class toolbar_tab_button_widget : public widget {
public:
    using super = widget;
    using delegate_type = radio_delegate;

    struct attributes_type {
        /** The label to show when the button is in the 'on' state.
         */
        observer<label> on_label = txt("on");

        /** The label to show when the button is in the 'off' state.
         */
        observer<label> off_label = txt("off");

        observer<alignment> alignment = alignment::top_center();

        attributes_type(attributes_type const&) noexcept = default;
        attributes_type(attributes_type&&) noexcept = default;
        attributes_type& operator=(attributes_type const&) noexcept = default;
        attributes_type& operator=(attributes_type&&) noexcept = default;

        template<toolbar_tab_button_widget_attribute... Attributes>
        explicit attributes_type(Attributes&&...attributes) noexcept
        {
            set_attributes<0>(std::forward<Attributes>(attributes)...);
        }

        template<size_t NumLabels>
        void set_attributes() noexcept
        {
        }

        template<size_t NumLabels, toolbar_tab_button_widget_attribute First, toolbar_tab_button_widget_attribute... Rest>
        void set_attributes(First&& first, Rest&&...rest) noexcept
        {
            if constexpr (forward_of<First, observer<hi::label>>) {
                if constexpr (NumLabels == 0) {
                    on_label = first;
                    off_label = std::forward<First>(first);

                } else if constexpr (NumLabels == 1) {
                    off_label.reset();
                    off_label = std::forward<First>(first);

                } else {
                    hi_static_no_default("Maximum two label attributes (on/off) are allowed on a toolbar-tab-button");
                }
                return set_attributes<NumLabels + 1>(std::forward<Rest>(rest)...);

            } else if constexpr (forward_of<First, observer<hi::alignment>>) {
                alignment = std::forward<First>(first);
                return set_attributes<NumLabels>(std::forward<Rest>(rest)...);

            } else {
                hi_static_no_default();
            }
        }
    };

    attributes_type attributes;

    /** The delegate that controls the button widget.
     */
    std::shared_ptr<delegate_type> delegate;

    hi_num_valid_arguments(consteval static, num_default_delegate_arguments, default_radio_delegate);
    hi_call_left_arguments(static, make_default_delegate, make_shared_ctad<default_radio_delegate>);
    hi_call_right_arguments(static, make_attributes, attributes_type);

    ~toolbar_tab_button_widget()
    {
        delegate->deinit(*this);
    }

    /** Construct a toolbar tab button widget.
     *
     * @param parent The parent widget that owns this radio button widget.
     * @param delegate The delegate to use to manage the state of the tab button widget.
     * @param attributes Different attributes used to configure the label's on the toolbar tab button:
     *                   a `label`, `alignment`. If one label is
     *                   passed it will be shown in all states. If two labels are passed
     *                   the first label is shown in on-state and the second for off-state.
     */
    toolbar_tab_button_widget(
        widget_intf const* parent,
        attributes_type attributes,
        std::shared_ptr<delegate_type> delegate) noexcept :
        super(parent), attributes(std::move(attributes)), delegate(std::move(delegate))
    {
        _on_label_widget = std::make_unique<label_widget>(
            this, this->attributes.on_label, this->attributes.alignment);
        _off_label_widget = std::make_unique<label_widget>(
            this, this->attributes.off_label, this->attributes.alignment);

        hi_axiom_not_null(this->delegate);
        this->delegate->init(*this);
        _delegate_cbt = this->delegate->subscribe([&] {
            set_value(this->delegate->state(*this));
        });
        _delegate_cbt();
    }

    /** Construct a toolbar tab button widget with a default radio delegate.
     *
     * @param parent The parent widget that owns this toggle widget.
     * @param args The arguments to the `default_radio_delegate`
     *                followed by arguments to `attributes_type`
     */
    template<typename... Args>
    toolbar_tab_button_widget(widget_intf const* parent, Args&&...args)
        requires(num_default_delegate_arguments<Args...>() != 0)
        :
        toolbar_tab_button_widget(
            parent,
            make_attributes<num_default_delegate_arguments<Args...>()>(std::forward<Args>(args)...),
            make_default_delegate<num_default_delegate_arguments<Args...>()>(std::forward<Args>(args)...))
    {
    }

    void request_redraw() const noexcept override
    {
        // A toolbar tab button draws a focus line across the whole toolbar
        // which is beyond it's own clipping rectangle. The parent is the toolbar
        // so it will include everything that needs to be redrawn.
        if (parent != nullptr) {
            parent->request_redraw();
        } else {
            super::request_redraw();
        }
    }

    /// @privatesection
    [[nodiscard]] box_constraints update_constraints() noexcept override
    {
        _layout = {};
        _on_label_constraints = _on_label_widget->update_constraints();
        _off_label_constraints = _off_label_widget->update_constraints();

        _label_constraints = max(_on_label_constraints, _off_label_constraints);

        // On left side a check mark, on right side short-cut. Around the label extra margin.
        auto const extra_size = extent2{theme().margin<float>() * 2.0f, theme().margin<float>()};
        return _label_constraints + extra_size;
    }

    void set_layout(widget_layout const& context) noexcept override
    {
        if (compare_store(_layout, context)) {
            auto const label_rectangle = aarectangle{
                theme().margin<float>(),
                0.0f,
                context.width() - theme().margin<float>() * 2.0f,
                context.height() - theme().margin<float>()};
            _on_label_shape = _off_label_shape =
                box_shape{_label_constraints, label_rectangle, theme().baseline_adjustment()};
        }

        _on_label_widget->set_mode(value() == widget_value::on ? widget_mode::display : widget_mode::invisible);
        _off_label_widget->set_mode(value() != widget_value::on ? widget_mode::display : widget_mode::invisible);

        _on_label_widget->set_layout(context.transform(_on_label_shape));
        _off_label_widget->set_layout(context.transform(_off_label_shape));
    }

    void draw(draw_context const& context) noexcept override
    {
        if (mode() > widget_mode::invisible and overlaps(context, layout())) {
            draw_toolbar_tab_button(context);
            _on_label_widget->draw(context);
            _off_label_widget->draw(context);
        }
    }

    [[nodiscard]] bool accepts_keyboard_focus(keyboard_focus_group group) const noexcept override
    {
        return mode() >= widget_mode::partial and to_bool(group & hi::keyboard_focus_group::toolbar);
    }

    [[nodiscard]] generator<widget_intf&> children(bool include_invisible) noexcept override
    {
        co_yield *_on_label_widget;
        co_yield *_off_label_widget;
    }

    [[nodiscard]] color background_color() const noexcept override
    {
        hi_axiom(loop::main().on_thread());
        if (phase() == widget_phase::pressed) {
            return theme().fill_color(_layout.layer + 2);
        } else {
            return super::background_color();
        }
    }

    [[nodiscard]] hitbox hitbox_test(point2 position) const noexcept override
    {
        hi_axiom(loop::main().on_thread());

        if (mode() >= widget_mode::partial and layout().contains(position)) {
            return {id, _layout.elevation, hitbox_type::button};
        } else {
            return {};
        }
    }

    void activate() noexcept
    {
        delegate->activate(*this);

        notifier();
    }

    bool handle_event(gui_event const& event) noexcept override
    {
        hi_axiom(loop::main().on_thread());

        switch (event.type()) {
        case gui_event_type::gui_activate:
            if (mode() >= widget_mode::partial) {
                activate();
                return true;
            }
            break;

        case gui_event_type::mouse_down:
            if (mode() >= widget_mode::partial and event.mouse().cause.left_button) {
                set_pressed(true);
                return true;
            }
            break;

        case gui_event_type::mouse_up:
            if (mode() >= widget_mode::partial and event.mouse().cause.left_button) {
                set_pressed(false);

                if (layout().rectangle().contains(event.mouse().position)) {
                    handle_event(gui_event_type::gui_activate);
                }
                return true;
            }
            break;

        default:;
        }

        return super::handle_event(event);
    }
    // @endprivatesection
protected:
    std::unique_ptr<label_widget> _on_label_widget;
    box_constraints _on_label_constraints;
    box_shape _on_label_shape;

    std::unique_ptr<label_widget> _off_label_widget;
    box_constraints _off_label_constraints;
    box_shape _off_label_shape;

    callback<void()> _delegate_cbt;

private:
    box_constraints _label_constraints;

    void draw_toolbar_tab_button(draw_context const& context) noexcept
    {
        // Draw the outline of the button across the clipping rectangle to clip the
        // bottom of the outline.
        auto const offset = theme().margin<float>() + theme().border_width();
        auto const outline_rectangle = aarectangle{0, -offset, layout().width(), layout().height() + offset};

        // The focus line will be drawn by the parent widget (toolbar_widget) at 0.5.
        auto const button_z = focus() ? translate_z(0.6f) : translate_z(0.0f);

        // clang-format off
        auto button_color = (phase() == widget_phase::hover or value() == widget_value::on) ?
            theme().fill_color(_layout.layer - 1) :
            theme().fill_color(_layout.layer);
        // clang-format on

        auto const corner_radii = hi::corner_radii(0.0f, 0.0f, theme().rounding_radius<float>(), theme().rounding_radius<float>());

        context.draw_box(
            layout(),
            button_z * outline_rectangle,
            button_color,
            focus() ? focus_color() : button_color,
            theme().border_width(),
            border_side::inside,
            corner_radii);
    }
};

} // namespace v1
} // namespace hi::v1
