// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "button_delegate.hpp"
#include "button_type.hpp"
#include "../observable.hpp"
#include <type_traits>
#include <memory>

namespace tt::inline v1 {

/** A default button delegate.
 *
 * The default button delegate manages the state of a button widget using
 * observable values.
 *
 * @tparam ButtonType The type of button this delegate manages, either a
 *         `button_type::radio` or `button_type::toggle`.
 * @tparam T The type of the observable value.
 */
template<button_type ButtonType, typename T>
class default_button_delegate : public button_delegate {
public:
    static_assert(ButtonType == button_type::radio or ButtonType == button_type::toggle);

    using value_type = T;
    static constexpr button_type button_type = ButtonType;
    static constexpr bool can_make_defaults =
        std::is_same_v<value_type, bool> or std::is_integral_v<value_type> or std::is_enum_v<value_type>;

    observable<value_type> value;
    observable<value_type> on_value;
    observable<value_type> off_value;

    /** Construct a delegate.
     *
     * @param value A value or observable-value used as a representation of the state.
     * @param on_value The value or observable-value that mean 'on'.
     * @param off_value The value or observable-value that mean 'off'.
     */
    default_button_delegate(auto &&value, auto &&on_value, auto &&off_value) noexcept :
        value(tt_forward(value)), on_value(tt_forward(on_value)), off_value(tt_forward(off_value))
    {
        // clang-format off
        _value_cbt = this->value.subscribe([&](auto...){ this->_notifier(); });
        _on_value_cbt = this->on_value.subscribe([&](auto...){ this->_notifier(); });
        _off_value_cbt = this->off_value.subscribe([&](auto...){ this->_notifier(); });
        // clang-format on
    }

    /** Construct a delegate.
     *
     * @param value A value or observable-value used as a representation of the state.
     * @param on_value The value or observable-value that mean 'on'.
     */
    default_button_delegate(auto &&value, auto &&on_value) noexcept
        requires(can_make_defaults or button_type == button_type::radio) :
        default_button_delegate(tt_forward(value), tt_forward(on_value), value_type{})
    {
    }

    /** Construct a delegate.
     *
     * @param value A value or observable-value used as a representation of the state.
     */
    default_button_delegate(auto &&value) noexcept requires(can_make_defaults) :
        default_button_delegate(tt_forward(value), value_type{1}, value_type{})
    {
    }

    /// @privatesection
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
    /// @endprivatesection
private:
    typename decltype(value)::token_type _value_cbt;
    typename decltype(on_value)::token_type _on_value_cbt;
    typename decltype(off_value)::token_type _off_value_cbt;
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

} // namespace tt::inline v1
