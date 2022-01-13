// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "abstract_button_widget.hpp"
#include "default_button_delegate.hpp"

namespace tt::inline v1 {

class toolbar_button_widget final : public abstract_button_widget {
public:
    using super = abstract_button_widget;
    using delegate_type = typename super::delegate_type;
    using callback_ptr_type = typename delegate_type::callback_ptr_type;

    template<typename Label>
    toolbar_button_widget(gui_window &window, widget *parent, Label &&label, std::weak_ptr<delegate_type> delegate) noexcept :
        toolbar_button_widget(window, parent, std::forward<Label>(label), weak_or_unique_ptr{std::move(delegate)})
    {
    }

    template<typename Label>
    toolbar_button_widget(gui_window &window, widget *parent, Label &&label) noexcept :
        toolbar_button_widget(window, parent, std::forward<Label>(label), std::make_unique<button_delegate>())
    {
    }

    /// @privatesection
    widget_constraints const &set_constraints() noexcept override;
    void set_layout(widget_layout const &layout) noexcept override;
    void draw(draw_context const &context) noexcept override;
    [[nodiscard]] bool accepts_keyboard_focus(keyboard_focus_group group) const noexcept override;
    [[nodiscard]] bool handle_event(command command) noexcept override;
    // @endprivatesection
private:
    template<typename Label>
    toolbar_button_widget(gui_window &window, widget *parent, Label &&label, weak_or_unique_ptr<delegate_type> delegate) noexcept
        :
        super(window, parent, std::move(delegate))
    {
        label_alignment = alignment::middle_left();
        set_label(std::forward<Label>(label));
    }

    void draw_toolbar_button(draw_context const &context) noexcept;
};

} // namespace tt::inline v1
