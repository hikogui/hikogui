// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/toolbar_button_widget.hpp Defines toolbar_button_widget.
 * @ingroup widgets
 */

#pragma once

#include "abstract_button_widget.hpp"

namespace hi { inline namespace v1 {

/** A momentary button used as a child in the toolbar.
 *
 * @ingroup widgets
 */
template<fixed_string Name = "">
class toolbar_button_widget final : public abstract_button_widget<Name / "toolbar-button"> {
public:
    using super = abstract_button_widget<Name / "toolbar-button">;
    using delegate_type = typename super::delegate_type;
    constexpr static auto prefix = super::prefix;

    toolbar_button_widget(
        widget *parent,
        std::shared_ptr<delegate_type> delegate,
        button_widget_attribute auto&&...attributes) noexcept :
        super(parent, std::move(delegate))
    {
        this->alignment = alignment::middle_left();
        this->set_attributes<0>(hi_forward(attributes)...);
    }

    toolbar_button_widget(widget *parent, button_widget_attribute auto&&...attributes) noexcept :
        toolbar_button_widget(parent, std::make_shared<button_delegate>(), hi_forward(attributes)...)
    {
    }

    /// @privatesection
    [[nodiscard]] box_constraints update_constraints() noexcept override
    {
        _label_constraints = super::update_constraints();

        // On left side a check mark, on right side short-cut. Around the label extra margin.
        hilet spacing = theme<prefix>.spacing_horizontal(this);
        hilet extra_size = extent2i{spacing * 2, spacing * 2};

        auto constraints = _label_constraints + extra_size;
        constraints.margins = 0;
        return constraints;
    }

    void set_layout(widget_layout const& context) noexcept override
    {
        if (compare_store(this->layout, context)) {
            hilet spacing = theme<prefix>.spacing_horizontal(this);
            hilet label_rectangle = aarectanglei{spacing, 0, context.width() - spacing * 2, context.height()};
            this->_on_label_shape = this->_off_label_shape = this->_other_label_shape =
                box_shape{_label_constraints, label_rectangle, theme<prefix>.cap_height(this)};
        }
        super::set_layout(context);
    }

    void draw(widget_draw_context const& context) noexcept override
    {
        if (*this->mode > widget_mode::invisible and overlaps(context, this->layout)) {
            draw_toolbar_button(context);
            this->draw_button(context);
        }
    }

    [[nodiscard]] bool accepts_keyboard_focus(keyboard_focus_group group) const noexcept override
    {
        return *this->mode >= widget_mode::partial and to_bool(group & hi::keyboard_focus_group::toolbar);
    }
    // @endprivatesection
private:
    box_constraints _label_constraints;

    void draw_toolbar_button(widget_draw_context const& context) noexcept
    {
        context.draw_box(
            this->layout,
            this->layout.rectangle(),
            theme<prefix>.background_color(this),
            theme<prefix>.border_color(this),
            theme<prefix>.border_width(this),
            border_side::inside);
    }
};

}} // namespace hi::v1
