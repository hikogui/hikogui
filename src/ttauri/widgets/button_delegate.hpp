// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "label_delegate.hpp"
#include "button_type.hpp"
#include "button_state.hpp"

namespace tt {

template<typename T, button_type ButtonType>
class button_delegate : public label_delegate {
public:
    static constexpr button_type button_type = ButtonType;

    using super = label_delegate;
    using value_type = T;
    using pressed_notifier_type = notifier<void()>;
    using pressed_callback_ptr_type = typename pressed_notifier_type::callback_ptr_type;

    button_delegate() noexcept : super(), _state(button_state::off), _value()
    {
        if constexpr (std::is_same_v<value_type, bool>) {
            _on_value = true;
            _off_value = false;
        } else if constexpr (std::is_integral_v<value_type> or std::is_enum_v<value_type>) {
            _on_value = static_cast<value_type>(1);
            _off_value = static_cast<value_type>(0);
        } else if constexpr (std::is_same_v<value_type, std::string>) {
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

    virtual void pressed(widget &sender) noexcept
    {
        if (button_type == button_type::toggle) {
            _value = (_value == _off_value) ? _on_value : _off_value;

        } else if (button_type == button_type::radio) {
            _value = _on_value;
        }
        value_changed();

        _pressed_notifier();
    }

    [[nodiscard]] virtual button_state state(widget const &sender) const noexcept
    {
        return _state;
    }

    [[nodiscard]] tt::label label(widget const &sender) const noexcept override
    {
        if (sender.lineage_matches_id("on_label")) {
            return _on_label;
        } else if (sender.lineage_matches_id("off_label")) {
            return _off_label;
        } else if (sender.lineage_matches_id("other_label")) {
            return _other_label;
        } else {
            tt_log_error("depricated");
            switch (_state) {
            case button_state::on: return _on_label;
            case button_state::off: return _off_label;
            case button_state::other: return _other_label;
            default: tt_no_default();
            }
        }
    }

    [[nodiscard]] bool visible(widget const &sender) const noexcept override
    {
        if (sender.lineage_matches_id("on_label")) {
            return state(sender) == button_state::on;
        } else if (sender.lineage_matches_id("off_label")) {
            return state(sender) == button_state::off;
        } else if (sender.lineage_matches_id("other_label")) {
            return state(sender) == button_state::other;
        } else {
            return super::visible(sender);
        }
    }

    void set_label(widget &sender, observable<tt::label> rhs) noexcept override
    {
        tt_no_default();
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
        this->_notifier(widget_update_level::constrain);
    }

    [[nodiscard]] virtual tt::label off_label(widget const &sender) const noexcept
    {
        return _off_label;
    }

    virtual void set_off_label(widget &sender, tt::label const &rhs) noexcept
    {
        _off_label = rhs;
        this->_notifier(widget_update_level::constrain);
    }

    [[nodiscard]] virtual tt::label other_label(widget const &sender) const noexcept
    {
        return _other_label;
    }

    virtual void set_other_label(widget &sender, tt::label const &rhs) noexcept
    {
        _other_label = rhs;
        this->_notifier(widget_update_level::constrain);
    }

protected:
    button_state _state;

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
        if (_value == _on_value) {
            _state = button_state::on;

        } else if (_value == _off_value) {
            _state = button_state::off;

        } else {
            _state = button_state::other;
        }

        this->_notifier(widget_update_level::layout);
    }
};

} // namespace tt
