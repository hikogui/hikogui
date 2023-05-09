// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/checkbox_delegate.hpp Defines checkbox_delegate and some default checkbox_delegate delegates.
 * @ingroup widget_delegates
 */

#pragma once

#include "../notifier.hpp"

namespace hi { inline namespace v1 {

/** A toggle delegate controls the state of a toggle widget.
 * @ingroup widget_delegates
 */
class checkbox_delegate {
public:
    using notifier_type = notifier<>;
    using callback_token = notifier_type::callback_token;
    using callback_proto = notifier_type::callback_proto;

    virtual ~checkbox_delegate() = default;

    virtual void init(widget& sender) noexcept {}

    virtual void deinit(widget& sender) noexcept {}

    /** Called when the button is pressed by the user.
     */
    virtual void activate(widget& sender) noexcept = 0;

    /** Used by the widget to check the state of the button.
     */
    [[nodiscard]] virtual widget_state state(widget const& sender) const noexcept
    {
        return widget_state::off;
    }

    /** Subscribe a callback for notifying the widget of a data change.
     */
    [[nodiscard]] callback_token
    subscribe(forward_of<callback_proto> auto&& callback, callback_flags flags = callback_flags::synchronous) noexcept
    {
        return _notifier.subscribe(hi_forward(callback), flags);
    }

protected:
    notifier_type _notifier;
};

/** A default toggle delegate.
 *
 * The default toggle delegate manages the state of a button widget using
 * observer values.
 *
 * @ingroup widget_delegates
 * @tparam T The type of the observer value.
 */
template<typename T>
class default_checkbox_delegate : public checkbox_delegate {
public:
    using value_type = T;

    observer<value_type> value;
    observer<value_type> on_value;
    observer<value_type> off_value;

    /** Construct a delegate.
     *
     * @param value A value or observer-value used as a representation of the state.
     * @param on_value The value or observer-value that mean 'on'.
     */
    default_checkbox_delegate(
        forward_of<observer<value_type>> auto&& value,
        forward_of<observer<value_type>> auto&& on_value,
        forward_of<observer<value_type>> auto&& off_value) noexcept :
        value(hi_forward(value)), on_value(hi_forward(on_value)), off_value(hi_forward(off_value))
    {
        // clang-format off
        _value_cbt = this->value.subscribe([&](auto...){ this->_notifier(); });
        _on_value_cbt = this->on_value.subscribe([&](auto...){ this->_notifier(); });
        _off_value_cbt = this->off_value.subscribe([&](auto...){ this->_notifier(); });
        // clang-format on
    }

    /** Construct a delegate.
     *
     * @param value A value or observer-value used as a representation of the state.
     * @param on_value The value or observer-value that mean 'on'.
     */
    default_checkbox_delegate(
        forward_of<observer<value_type>> auto&& value,
        forward_of<observer<value_type>> auto&& on_value) noexcept
        requires default_values_v<value_type>
        : default_checkbox_delegate(hi_forward(value), hi_forward(on_value), default_values<value_type>::off)
    {
    }

    /** Construct a delegate.
     *
     * @param value A value or observer-value used as a representation of the state.
     */
    default_checkbox_delegate(forward_of<observer<value_type>> auto&& value) noexcept
        requires default_values_v<value_type>
        : default_checkbox_delegate(hi_forward(value), default_values<value_type>::on, default_values<value_type>::off)
    {
    }

    /// @privatesection
    [[nodiscard]] widget_state state(widget const& sender) const noexcept override
    {
        if (*value == *on_value) {
            return widget_state::on;
        } else {
            return widget_state::off;
        }
    }

    void activate(widget& sender) noexcept override
    {
        if (*value == *on_value) {
            value = *off_value;
        } else {
            value = *on_value;
        }
    }
    /// @endprivatesection
private:
    typename decltype(value)::callback_token _value_cbt;
    typename decltype(on_value)::callback_token _on_value_cbt;
    typename decltype(off_value)::callback_token _off_value_cbt;
};

/** Make a shared pointer to a toggle-button delegate.
 *
 * @ingroup widget_delegates
 * @see default_checkbox_button_delegate
 * @param value A value or observer-value used as a representation of the state.
 * @param args an optional on-value followed by an optional off-value.
 * @return A shared_ptr to a button delegate.
 */
[[nodiscard]] std::shared_ptr<checkbox_delegate> make_default_checkbox_delegate(auto&& value, auto&&...args) noexcept
    requires requires {
                 default_checkbox_delegate<observer_decay_t<decltype(value)>>{hi_forward(value), hi_forward(args)...};
             }
{
    return std::make_shared<default_checkbox_delegate<observer_decay_t<decltype(value)>>>(
        hi_forward(value), hi_forward(args)...);
}

}} // namespace hi::v1
