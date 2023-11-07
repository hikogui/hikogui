// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/toolbar_button_widget.hpp Defines toolbar_button_widget.
 * @ingroup widgets
 */

module;
#include "../macros.hpp"


export module hikogui_widgets_toolbar_button_widget;
import hikogui_widgets_abstract_button_widget;

export namespace hi { inline namespace v1 {

/** A momentary button used as a child in the toolbar.
 *
 * @ingroup widgets
 */
class toolbar_button_widget final : public abstract_button_widget {
public:
    using super = abstract_button_widget;
    using delegate_type = typename super::delegate_type;

    toolbar_button_widget(
        not_null<widget_intf const *> parent,
        std::shared_ptr<delegate_type> delegate,
        button_widget_attribute auto&&...attributes) noexcept :
        super(parent, std::move(delegate))
    {
        alignment = alignment::middle_left();
        set_attributes<0>(hi_forward(attributes)...);
    }

    toolbar_button_widget(not_null<widget_intf const *> parent, button_widget_attribute auto&&...attributes) noexcept :
        toolbar_button_widget(parent, std::make_shared<button_delegate>(), hi_forward(attributes)...)
    {
    }

    /// @privatesection
    [[nodiscard]] box_constraints update_constraints() noexcept override
    {
        _label_constraints = super::update_constraints();

        // On left side a check mark, on right side short-cut. Around the label extra margin.
        hilet extra_size = extent2{theme().margin<float>() * 2.0f, theme().margin<float>() * 2.0f};

        auto constraints = _label_constraints + extra_size;
        constraints.margins = 0;
        return constraints;
    }
    void set_layout(widget_layout const& context) noexcept override
    {
        if (compare_store(_layout, context)) {
            hilet label_rectangle =
                aarectangle{theme().margin<float>(), 0.0f, context.width() - theme().margin<float>() * 2.0f, context.height()};
            _on_label_shape = _off_label_shape = _other_label_shape =
                box_shape{_label_constraints, label_rectangle, theme().baseline_adjustment()};
        }
        super::set_layout(context);
    }
    void draw(draw_context const& context) noexcept override
    {
        if (*mode > widget_mode::invisible and overlaps(context, layout())) {
            draw_toolbar_button(context);
            draw_button(context);
        }
    }
    [[nodiscard]] bool accepts_keyboard_focus(keyboard_focus_group group) const noexcept override
    {
        return *mode >= widget_mode::partial and to_bool(group & hi::keyboard_focus_group::toolbar);
    }
    // @endprivatesection
private:
    box_constraints _label_constraints;

    void draw_toolbar_button(draw_context const& context) noexcept
    {
        hilet border_color = *focus ? focus_color() : color::transparent();
        context.draw_box(
            layout(), layout().rectangle(), background_color(), border_color, theme().border_width(), border_side::inside);
    }
};

}} // namespace hi::v1
