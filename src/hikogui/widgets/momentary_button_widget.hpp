// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/momentary_button_widget.hpp Defines momentary_button_widget.
 * @ingroup widgets
 */

#pragma once

#include "abstract_button_widget.hpp"

namespace hi { inline namespace v1 {

/** A momentary button widget.
 * @ingroup widgets
 */
template<fixed_string Name = "">
class momentary_button_widget final : public abstract_button_widget<Name / "momentary-button"> {
public:
    using super = abstract_button_widget<Name / "momentary-button">;
    using delegate_type = typename super::delegate_type;
    constexpr static auto prefix = super::prefix;

    momentary_button_widget(
        widget *parent,
        std::shared_ptr<delegate_type> delegate,
        button_widget_attribute auto&&...attributes) noexcept :
        super(parent, std::move(delegate))
    {
        this->alignment = alignment::middle_center();
        this->set_attributes<0>(hi_forward(attributes)...);
    }

    momentary_button_widget(widget *parent, button_widget_attribute auto&&...attributes) noexcept :
        momentary_button_widget(parent, std::make_shared<delegate_type>(), hi_forward(attributes)...)
    {
    }

    /// @privatesection
    [[nodiscard]] box_constraints update_constraints() noexcept override
    {
        _label_constraints = super::update_constraints();

        auto constraints = _label_constraints;
        constraints.minimum.width() += _label_constraints.margins.left() + _label_constraints.margins.right();
        constraints.preferred.width() += _label_constraints.margins.left() + _label_constraints.margins.right();
        constraints.maximum.width() += _label_constraints.margins.left() + _label_constraints.margins.right();
        constraints.minimum.height() += _label_constraints.margins.bottom() + _label_constraints.margins.top();
        constraints.preferred.height() += _label_constraints.margins.bottom() + _label_constraints.margins.top();
        constraints.maximum.height() += _label_constraints.margins.bottom() + _label_constraints.margins.top();
        constraints.margins = theme<prefix>.margin(this);
        return constraints;
    }

    void set_layout(widget_layout const& context) noexcept override
    {
        if (compare_store(this->layout, context)) {
            hilet label_rectangle = aarectanglei{context.size()} - _label_constraints.margins;

            this->_on_label_shape = this->_off_label_shape = this->_other_label_shape =
                box_shape{_label_constraints, label_rectangle, theme<prefix>.cap_height(this)};
        }
        super::set_layout(context);
    }

    void draw(widget_draw_context& context) noexcept override
    {
        if (*this->mode > widget_mode::invisible and overlaps(context, this->layout)) {
            draw_label_button(context);
            this->draw_button(context);
        }
    }
    /// @endprivatesection
private:
    box_constraints _label_constraints;

    void draw_label_button(widget_draw_context& context) noexcept
    {
        // Move the border of the button in the middle of a pixel.
        context.draw_box(
            this->layout,
            this->layout.rectangle(),
            theme<prefix>.background_color(this),
            theme<prefix>.border_color(this),
            theme<prefix>.border_width(this),
            border_side::inside,
            theme<prefix>.border_radius(this));
    }
};

}} // namespace hi::v1
