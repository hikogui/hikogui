// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "widget.hpp"
#include "button_delegate.hpp"
#include "label_widget.hpp"
#include "button_type.hpp"
#include "../animator.hpp"
#include "../l10n.hpp"
#include "../notifier.hpp"
#include "../weak_or_unique_ptr.hpp"
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>

namespace tt::inline v1 {

class abstract_button_widget : public widget {
public:
    using super = widget;
    using delegate_type = button_delegate;

    /** The label to show when the button is in the 'on' state.
     */
    observable<label> on_label = l10n("on");

    /** The label to show when the button is in the 'off' state.
     */
    observable<label> off_label = l10n("off");

    /** The label to show when the button is in the 'other' state.
     */
    observable<label> other_label = l10n("other");

    /** The alignment of the on/off/other label.
     */
    observable<alignment> label_alignment;

    notifier<void()> pressed;

    /** Set on/off/other labels of the button to the same value.
     */
    template<typename Label>
    void set_label(Label const &rhs) noexcept
    {
        tt_axiom(is_gui_thread());
        on_label = rhs;
        off_label = rhs;
        other_label = rhs;
    }

    /** Get the current state of the button.
     * @return The state of the button: on / off / other.
     */
    [[nodiscard]] button_state state() const noexcept
    {
        tt_axiom(is_gui_thread());
        if (auto delegate = _delegate.lock()) {
            return delegate->state(*this);
        } else {
            return button_state::off;
        }
    }

    /// @privatesection
    [[nodiscard]] pmr::generator<widget *> children(std::pmr::polymorphic_allocator<> &) const noexcept override
    {
        co_yield _on_label_widget.get();
        co_yield _off_label_widget.get();
        co_yield _other_label_widget.get();
    }

    [[nodiscard]] color background_color() const noexcept override;
    [[nodiscard]] hitbox hitbox_test(point3 position) const noexcept final;
    [[nodiscard]] bool accepts_keyboard_focus(keyboard_focus_group group) const noexcept override;
    void activate() noexcept;
    [[nodiscard]] bool handle_event(command command) noexcept override;
    [[nodiscard]] bool handle_event(mouse_event const &event) noexcept final;
    /// @endprivatesection
protected:
    aarectangle _label_rectangle;
    std::unique_ptr<label_widget> _on_label_widget;
    std::unique_ptr<label_widget> _off_label_widget;
    std::unique_ptr<label_widget> _other_label_widget;

    bool _pressed = false;
    weak_or_unique_ptr<delegate_type> _delegate;
    notifier<>::token_type _delegate_cbt;

    ~abstract_button_widget();
    abstract_button_widget(gui_window &window, widget *parent, weak_or_unique_ptr<delegate_type> delegate) noexcept;

    widget_constraints set_constraints_button() const noexcept;
    void set_layout_button(widget_layout const &context) noexcept;
    void draw_button(draw_context const &context) noexcept;
};

} // namespace tt::inline v1
