// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "widget.hpp"
#include "button_delegate.hpp"
#include "label_widget.hpp"
#include "button_type.hpp"
#include "../animator.hpp"
#include "../i18n/translate.hpp"
#include "../notifier.hpp"
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>

namespace hi::inline v1 {

class abstract_button_widget : public widget {
public:
    using super = widget;
    using delegate_type = button_delegate;

    /** The delegate that controls the button widget.
     */
    std::shared_ptr<delegate_type> delegate;

    /** The label to show when the button is in the 'on' state.
     */
    observer<label> on_label = tr("on");

    /** The label to show when the button is in the 'off' state.
     */
    observer<label> off_label = tr("off");

    /** The label to show when the button is in the 'other' state.
     */
    observer<label> other_label = tr("other");

    /** The alignment of the button and on/off/other label.
     */
    observer<alignment> alignment;

    notifier<void()> pressed;

    ~abstract_button_widget();
    
    abstract_button_widget(gui_window& window, widget *parent, std::shared_ptr<delegate_type> delegate) noexcept;

    /** Set on/off/other labels of the button to the same value.
     */
    template<typename Label>
    void set_label(Label const &rhs) noexcept
    {
        hi_axiom(is_gui_thread());
        on_label = rhs;
        off_label = rhs;
        other_label = rhs;
    }

    /** Get the current state of the button.
     * @return The state of the button: on / off / other.
     */
    [[nodiscard]] button_state state() const noexcept
    {
        hi_axiom(is_gui_thread());
        hi_axiom(delegate != nullptr);
        return delegate->state(*this);
    }

    /// @privatesection
    [[nodiscard]] generator<widget *> children() const noexcept override
    {
        co_yield _on_label_widget.get();
        co_yield _off_label_widget.get();
        co_yield _other_label_widget.get();
    }

    [[nodiscard]] color background_color() const noexcept override;
    [[nodiscard]] hitbox hitbox_test(point3 position) const noexcept final;
    [[nodiscard]] bool accepts_keyboard_focus(keyboard_focus_group group) const noexcept override;
    void activate() noexcept;
    bool handle_event(gui_event const& event) noexcept override;
    /// @endprivatesection
protected:
    aarectangle _label_rectangle;
    std::unique_ptr<label_widget> _on_label_widget;
    std::unique_ptr<label_widget> _off_label_widget;
    std::unique_ptr<label_widget> _other_label_widget;

    bool _pressed = false;
    notifier<>::token_type _delegate_cbt;

    widget_constraints set_constraints_button() const noexcept;
    void set_layout_button(widget_layout const &context) noexcept;
    void draw_button(draw_context const &context) noexcept;
};

} // namespace hi::inline v1
