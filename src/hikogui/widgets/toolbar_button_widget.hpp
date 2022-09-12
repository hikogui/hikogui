// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "abstract_button_widget.hpp"
#include "default_button_delegate.hpp"

namespace hi::inline v1 {

class toolbar_button_widget final : public abstract_button_widget {
public:
    using super = abstract_button_widget;
    using delegate_type = typename super::delegate_type;

    toolbar_button_widget(
        gui_window& window,
        widget *parent,
        std::shared_ptr<delegate_type> delegate,
        forward_of<observer<hi::label>> auto&& label) noexcept :
        super(window, parent, std::move(delegate))
    {
        alignment = alignment::middle_left();
        set_label(hi_forward(label));
    }

    toolbar_button_widget(gui_window& window, widget *parent, forward_of<observer<hi::label>> auto && label) noexcept :
        toolbar_button_widget(window, parent, std::make_shared<button_delegate>(), hi_forward(label))
    {
    }

    /// @privatesection
    widget_constraints const& set_constraints() noexcept override;
    void set_layout(widget_layout const& layout) noexcept override;
    void draw(draw_context const& context) noexcept override;
    [[nodiscard]] bool accepts_keyboard_focus(keyboard_focus_group group) const noexcept override;
    // @endprivatesection
private:
    void draw_toolbar_button(draw_context const& context) noexcept;
};

} // namespace hi::inline v1
