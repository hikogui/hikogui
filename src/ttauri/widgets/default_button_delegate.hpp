// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "button_delegate.hpp"
#include "button_type.hpp"
#include "../observable.hpp"
#include <type_traits>
#include <memory>

namespace tt {

template<button_type ButtonType, typename T>
class default_button_delegate : public button_delegate {
public:
    static_assert(ButtonType == button_type::radio or ButtonType == button_type::toggle);

    using value_type = T;
    static constexpr bool can_make_defaults =
        std::is_same_v<value_type, bool> or std::is_integral_v<value_type> or std::is_enum_v<value_type>;

    static constexpr button_type button_type = ButtonType;

    observable<value_type> value;
    observable<value_type> on_value;
    observable<value_type> off_value;

    template<typename Value, typename OnValue, typename OffValue>
    default_button_delegate(Value &&value, OnValue &&on_value, OffValue &&off_value) noexcept :
        value(std::forward<Value>(value)), on_value(std::forward<OnValue>(on_value)), off_value(std::forward<OnValue>(off_value))
    {
    }

    template<typename Value, typename OnValue>
    default_button_delegate(Value &&value, OnValue &&on_value) noexcept
        requires(can_make_defaults or button_type == button_type::radio) :
        default_button_delegate(std::forward<Value>(value), std::forward<OnValue>(on_value), value_type{})
    {
    }

    template<typename Value>
    default_button_delegate(Value &&value) noexcept requires(can_make_defaults) :
        default_button_delegate(std::forward<Value>(value), static_cast<value_type>(1), value_type{})
    {
    }

    default_button_delegate() noexcept requires(can_make_defaults) :
        default_button_delegate(value_type{}, static_cast<value_type>(1), value_type{})
    {
    }

    callback_ptr_type subscribe(abstract_button_widget &sender, callback_ptr_type const &callback_ptr) noexcept override
    {
        value.subscribe(callback_ptr);
        on_value.subscribe(callback_ptr);
        off_value.subscribe(callback_ptr);
        return callback_ptr;
    }

    void unsubscribe(abstract_button_widget &sender, callback_ptr_type const &callback_ptr) noexcept override
    {
        value.unsubscribe(callback_ptr);
        on_value.unsubscribe(callback_ptr);
        off_value.unsubscribe(callback_ptr);
    }

    [[nodiscard]] button_state state(abstract_button_widget const &sender) const noexcept override
    {
        if (value == on_value) {
            return button_state::on;
        } else if (value == off_value) {
            return button_state::off;
        } else {
            return button_state::other;
        }
    }

    void activate(abstract_button_widget &sender) noexcept override
    {
        if constexpr (button_type == button_type::toggle) {
            if (value == off_value) {
                value = *on_value;
            } else {
                value = *off_value;
            }
        } else if constexpr (button_type == button_type::radio) {
            value = *on_value;
        }
    }
};

template<button_type ButtonType, typename Value, typename... Args>
default_button_delegate(Value &&, Args &&...)
    -> default_button_delegate<ButtonType, observable_argument_t<std::remove_cvref_t<Value>>>;

template<button_type ButtonType, typename Value, typename... Args>
std::unique_ptr<button_delegate> make_unique_default_button_delegate(Value &&value, Args &&...args) noexcept
{
    using value_type = observable_argument_t<std::remove_cvref_t<Value>>;
    return std::make_unique<default_button_delegate<ButtonType, value_type>>(
        std::forward<Value>(value), std::forward<Args>(args)...);
}

} // namespace tt