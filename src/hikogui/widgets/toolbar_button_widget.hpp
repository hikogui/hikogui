// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/toolbar_button_widget.hpp Defines toolbar_button_widget.
 * @ingroup widgets
 */

#pragma once

#include "abstract_button_widget.hpp"
#include "../macros.hpp"
#include <stop_token>

hi_export_module(hikogui.widgets.toolbar_button_widget);

hi_export namespace hi { inline namespace v1 {

/** A momentary button used as a child in the toolbar.
 *
 * @ingroup widgets
 */
class toolbar_button_widget : public abstract_button_widget {
public:
    using super = abstract_button_widget;
    using delegate_type = typename super::delegate_type;

    template<button_widget_attribute... Attributes>
    toolbar_button_widget(
        widget_intf const* parent,
        std::shared_ptr<delegate_type> delegate,
        Attributes&&...attributes) noexcept :
        super(parent, std::move(delegate))
    {
        alignment = alignment::middle_left();
        set_attributes<0>(std::forward<Attributes>(attributes)...);
    }

    template<button_widget_attribute... Attributes>
    toolbar_button_widget(widget_intf const* parent, Attributes&&...attributes) noexcept :
        toolbar_button_widget(parent, std::make_shared<button_delegate>(), std::forward<Attributes>(attributes)...)
    {
    }

    [[nodiscard]] std::stop_token get_stop_token() const noexcept
    {
        return _stop_source.get_token();
    }

    template<typename Awaiter>
    task<> wait_for(Awaiter &&awaiter) noexcept
    {
        co_await std::forward<Awaiter>(awaiter);
        set_value(widget_value::off);
    }

    /// @privatesection
    [[nodiscard]] box_constraints update_constraints() noexcept override
    {
        _label_constraints = super::update_constraints();

        // On left side a check mark, on right side short-cut. Around the label extra margin.
        auto const extra_size = extent2{theme().margin<float>() * 2.0f, theme().margin<float>() * 2.0f};

        auto constraints = _label_constraints + extra_size;
        constraints.margins = 0;
        return constraints;
    }
    void set_layout(widget_layout const& context) noexcept override
    {
        if (compare_store(_layout, context)) {
            auto const label_rectangle =
                aarectangle{theme().margin<float>(), 0.0f, context.width() - theme().margin<float>() * 2.0f, context.height()};
            _on_label_shape = _off_label_shape = _other_label_shape =
                box_shape{_label_constraints, label_rectangle, theme().baseline_adjustment()};
        }
        super::set_layout(context);
    }
    void draw(draw_context const& context) noexcept override
    {
        if (mode() > widget_mode::invisible and overlaps(context, layout())) {
            draw_toolbar_button(context);
            draw_button(context);
        }
    }
    [[nodiscard]] bool accepts_keyboard_focus(keyboard_focus_group group) const noexcept override
    {
        return mode() >= widget_mode::partial and to_bool(group & hi::keyboard_focus_group::toolbar);
    }
    // @endprivatesection
private:
    box_constraints _label_constraints;
    std::stop_source _stop_source;

    void draw_toolbar_button(draw_context const& context) noexcept
    {
        auto const border_color = focus() ? focus_color() : color::transparent();
        context.draw_box(
            layout(), layout().rectangle(), background_color(), border_color, theme().border_width(), border_side::inside);
    }
};

}} // namespace hi::v1
