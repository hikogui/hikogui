// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/momentary_button_widget.hpp Defines momentary_button_widget.
 * @ingroup widgets
 */

#pragma once

#include "abstract_button_widget.hpp"
#include "../macros.hpp"

hi_export_module(hikogui.widgets.momentary_button_widget);

hi_export namespace hi {
inline namespace v1 {

/** A momentary button widget.
 * @ingroup widgets
 */
class momentary_button_widget : public abstract_button_widget {
public:
    using super = abstract_button_widget;
    using delegate_type = typename super::delegate_type;

    template<button_widget_attribute... Attributes>
    momentary_button_widget(std::shared_ptr<delegate_type> delegate, Attributes&&... attributes) noexcept :
        super(std::move(delegate))
    {
        alignment = alignment::middle_center();
        set_attributes<0>(std::forward<Attributes>(attributes)...);

        style.set_name("momentary-button");
    }

    template<button_widget_attribute... Attributes>
    momentary_button_widget(Attributes&&... attributes) noexcept :
        momentary_button_widget(std::make_shared<delegate_type>(), std::forward<Attributes>(attributes)...)
    {
    }

    /// @privatesection
    [[nodiscard]] box_constraints update_constraints() noexcept override
    {
        _label_constraints = super::update_constraints();
        auto const padding = max(style.padding_px, _label_constraints.margins);

        auto r = _label_constraints + padding;
        r.margins = style.margins_px;
        r.baseline = embed(_label_constraints.baseline, style.baseline_priority, padding.bottom(), padding.top());
        return r;
    }

    void set_layout(widget_layout const& context) noexcept override
    {
        super::set_layout(context);

        auto const padding = max(style.padding_px, _label_constraints.margins);
        auto const label_rectangle = context.rectangle() - padding;

        _on_label_shape = _off_label_shape = _other_label_shape =
            box_shape{label_rectangle, lift(context.baseline(), padding.bottom(), padding.top())};
    }
    void draw(draw_context const& context) noexcept override
    {
        if (mode() > widget_mode::invisible and overlaps(context, layout())) {
            draw_label_button(context);
            draw_button(context);
        }
    }
    /// @endprivatesection
private:
    box_constraints _label_constraints;

    void draw_label_button(draw_context const& context) noexcept
    {
        // Move the border of the button in the middle of a pixel.
        context.draw_box(
            layout(),
            layout().rectangle(),
            style.background_color,
            style.border_color,
            style.border_width_px,
            border_side::inside,
            style.border_radius_px);
    }
};

} // namespace v1
} // namespace hi::v1
