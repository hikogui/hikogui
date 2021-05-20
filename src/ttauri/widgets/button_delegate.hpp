// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "widget_delegate.hpp"

namespace tt {

enum class button_state { off, on, other };

enum class button_type { momentary, toggle, radio };

template<typename T>
class button_delegate : public widget_delegate {
public:
    using value_type = T;
    using pressed_notifier_type = notifier<void()>;
    using pressed_callback_ptr_type = typename pressed_notifier_type::callback_ptr_type;

    button_delegate() noexcept :
        widget_delegate(), _state(button_state::off), _value()
    {
        if constexpr (std::is_same_v<value_type,bool>) {
            _on_value = true;
            _off_value = false;
        } else if constexpr (std::is_integral_v<value_type> or std::is_enum_v<value_type>) {
            _on_value = static_cast<value_type>(1);
            _off_value = static_cast<value_type>(0);
        } else if constexpr (std::is_same_v<value_type,std::string>) {
            _on_value = std::string{"true"};
            _off_value = std::string{};
        } else {
            tt_static_not_implemented();
        }

        _value_callback = _value.subscribe([this](auto...) {
            this->value_changed();
        });

        value_changed();
    }

    virtual void pressed(widget &sender, button_type type) noexcept
    {
        if (type == button_type::toggle) {
            _value = (_value == _off_value) ? _on_value : _off_value;

        } else if (type == button_type::radio) {
            _value = _on_value;
        }
        value_changed();

        _pressed_notifier();
    }

    [[nodiscard]] virtual button_state state(widget const &sender) const noexcept
    {
        return _state;
    }

    [[nodiscard]] virtual tt::label label(widget const &sender) const noexcept
    {
        return _label;
    }

    template<typename Callback>
    [[nodiscard]] pressed_callback_ptr_type subscribe_pressed(widget &sender, Callback &&callback) noexcept
    {
        return _pressed_notifier.subscribe(std::forward<Callback>(callback));
    }

    void unsubscribe_pressed(widget &sender, pressed_callback_ptr_type &ptr) noexcept
    {
        return _pressed_notifier.unsubscribe(ptr);
    }

    [[nodiscard]] virtual value_type value(widget const &sender) const noexcept
    {
        return *_value;
    }

    virtual void set_value(widget &sender, value_type const &rhs) noexcept
    {
        _value = rhs;
    }

    virtual void set_value(widget &sender, observable<value_type> rhs) noexcept
    {
        _value = std::move(rhs);
    }

    [[nodiscard]] virtual value_type on_value(widget const &sender) const noexcept
    {
        return _on_value;
    }

    virtual void set_on_value(widget &sender, value_type const &rhs) noexcept
    {
        _on_value = rhs;
        value_changed();
    }

    [[nodiscard]] virtual value_type off_value(widget const &sender) const noexcept
    {
        return _off_value;
    }

    virtual void set_off_value(widget &sender, value_type const &rhs) noexcept
    {
        _off_value = rhs;
        value_changed();
    }

    [[nodiscard]] virtual tt::label on_label(widget const &sender) const noexcept
    {
        return _on_label;
    }

    virtual void set_on_label(widget &sender, tt::label const &rhs) noexcept
    {
        _on_label = rhs;
        value_changed();
    }

    [[nodiscard]] virtual tt::label off_label(widget const &sender) const noexcept
    {
        return _off_label;
    }

    virtual void set_off_label(widget &sender, tt::label const &rhs) noexcept
    {
        _off_label = rhs;
        value_changed();
    }

    [[nodiscard]] virtual tt::label other_label(widget const &sender) const noexcept
    {
        return _other_label;
    }

    virtual void set_other_label(widget &sender, tt::label const &rhs) noexcept
    {
        _other_label = rhs;
        value_changed();
    }

protected:
    button_state _state;
    tt::label _label;

    value_type _on_value;
    value_type _off_value;

    tt::label _on_label;
    tt::label _off_label;
    tt::label _other_label;

    observable<T> _value;
    typename decltype(_value)::callback_ptr_type _value_callback;

    pressed_notifier_type _pressed_notifier;

    void value_changed() noexcept
    {
        auto label_changed = false;

        if (_value == _on_value) {
            _state = button_state::on;
            label_changed |= compare_then_assign(_label, _on_label);

        } else if (_value == _off_value) {
            _state = button_state::off;
            label_changed |= compare_then_assign(_label, _off_label);

        } else {
            _state = button_state::other;
            label_changed |= compare_then_assign(_label, _other_label);
        }

        this->_notifier(label_changed ? widget_update_level::constrain : widget_update_level::redraw);
    }
};

} // namespace tt
