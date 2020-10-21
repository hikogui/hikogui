// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "Widget.hpp"

namespace tt {

/** An abstract button widget.
 * This widgets implements the behaviour for a widget where its whole
 * area is clickable, accepts and responds to keyboard activate commands.
 */
class abstract_button_widget : public Widget {
public:
    using notifier_type = notifier<>;
    using callback_type = typename notifier_type::callback_type;
    using callback_id_type = typename notifier_type::callback_id_type;

    [[nodiscard]] abstract_button_widget(Window &window, Widget *parent) :
        Widget(window, parent)
    {
    }

    [[nodiscard]] bool accepts_focus() const noexcept final
    {
        tt_assume(mutex.is_locked_by_current_thread());
        return *enabled;
    }

    [[nodiscard]] bool handle_command(command command) noexcept final
    {
        ttlet lock = std::scoped_lock(mutex);
        auto handled = Widget::handle_command(command);

        if (*enabled) {
            if (command == command::gui_activate) {
                handled = true;
                _notifier();
            }
        }

        return handled;
    }

    [[nodiscard]] bool handle_mouse_event(MouseEvent const &event) noexcept final
    {
        ttlet lock = std::scoped_lock(mutex);
        auto handled = Widget::handle_mouse_event(event);

        if (event.cause.leftButton) {
            handled = true;
            if (*enabled) {
                if (compare_then_assign(_pressed, static_cast<bool>(event.down.leftButton))) {
                    window.requestRedraw = true;
                }

                if (event.type == MouseEvent::Type::ButtonUp && p_window_rectangle.contains(event.position)) {
                    handled |= handle_command(command::gui_activate);
                }
            }
        }
        return handled;
    }

    [[nodiscard]] HitBox hitbox_test(vec window_position) const noexcept final
    {
        ttlet lock = std::scoped_lock(mutex);

        if (p_window_clipping_rectangle.contains(window_position)) {
            return HitBox{this, p_draw_layer, *enabled ? HitBox::Type::Button : HitBox::Type::Default};
        } else {
            return HitBox{};
        }
    }

    [[nodiscard]] callback_id_type subscribe(callback_type const &callback) noexcept
    {
        return _notifier.subscribe(callback);
    }

    [[nodiscard]] callback_id_type subscribe(callback_type &&callback) noexcept
    {
        return _notifier.subscribe(std::move(callback));
    }

    void unsubscribe(callback_id_type id) noexcept
    {
        return _notifier.unsubscribe(id);
    }

protected:
    /** The button is in a pressed state.
     */
    bool _pressed = false;

private:
    notifier_type _notifier;
};

}
