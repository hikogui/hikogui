// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "abstract_button_widget.hpp"

namespace hi::inline v1 {

class momentary_button_widget final : public abstract_button_widget {
public:
    using super = abstract_button_widget;
    using delegate_type = typename super::delegate_type;

    momentary_button_widget(
        gui_window& window,
        widget *parent,
        std::shared_ptr<delegate_type> delegate) noexcept :
        super(window, parent, std::move(delegate))
    {
        alignment = alignment::middle_center();
    }

    momentary_button_widget(gui_window& window, widget *parent, button_widget_attribute auto&&...attributes) noexcept :
        momentary_button_widget(window, parent, std::make_shared<delegate_type>())
    {
        set_attributes<0>(hi_forward(attributes)...);
    }

    /// @privatesection
    widget_constraints const& set_constraints() noexcept override;
    void set_layout(widget_layout const& layout) noexcept override;
    void draw(draw_context const& context) noexcept override;
    /// @endprivatesection
private:
    void draw_label_button(draw_context const& context) noexcept;
};

} // namespace hi::inline v1
