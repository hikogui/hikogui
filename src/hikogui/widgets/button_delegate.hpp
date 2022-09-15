// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/button_delegate.hpp Defines button_delegate and some default button delegates.
 * @ingroup widget_delegates
 */

#pragma once

#include "../notifier.hpp"
#include "../observer.hpp"
#include <type_traits>
#include <memory>

namespace hi { inline namespace v1 {

class abstract_button_widget;

/** The state of a button.
 * @ingroup widget_delegates
 */
enum class button_state {
    /** The 'off' state of a button.
     */
    off,

    /** The 'on' state of a button.
     */
    on,

    /** The other state of a button.
     *
     * For checkboxes the 'other' state is when the value it represents is
     * neither 'on' or 'off'. Examples off this is when the checkbox is a parent
     * in a tree structure, where 'other' represents that its children have
     * different values.
     */
    other
};

/** A button delegate controls the state of a button widget.
 * @ingroup widget_delegates
 */
class button_delegate {
public:
    using notifier_type = notifier<>;
    using callback_token = notifier_type::callback_token;
    using callback_proto = notifier_type::callback_proto;

    virtual ~button_delegate() = default;

    virtual void init(abstract_button_widget& sender) noexcept {}

    virtual void deinit(abstract_button_widget& sender) noexcept {}

    /** Called when the button is pressed by the user.
     */
    virtual void activate(abstract_button_widget& sender) noexcept {};

    /** Used by the widget to check the state of the button.
     */
    [[nodiscard]] virtual button_state state(abstract_button_widget const& sender) const noexcept
    {
        return button_state::off;
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

/** A default radio button delegate.
 *
 * The default radio button delegate manages the state of a button widget using
 * observer values.
 *
 * @ingroup widget_delegates
 * @tparam T The type of the observer value.
 */
template<typename T>
class default_radio_button_delegate : public button_delegate {
public:
    using value_type = T;

    observer<value_type> value;
    observer<value_type> on_value;

    /** Construct a delegate.
     *
     * @param value A value or observer-value used as a representation of the state.
     * @param on_value The value or observer-value that mean 'on'.
     */
    default_radio_button_delegate(
        forward_of<observer<value_type>> auto&& value,
        forward_of<observer<value_type>> auto&& on_value) noexcept :
        value(hi_forward(value)), on_value(hi_forward(on_value))
    {
        // clang-format off
        _value_cbt = this->value.subscribe([&](auto...){ this->_notifier(); });
        _on_value_cbt = this->on_value.subscribe([&](auto...){ this->_notifier(); });
        // clang-format on
    }

    /// @privatesection
    [[nodiscard]] button_state state(abstract_button_widget const& sender) const noexcept override
    {
        if (*value == *on_value) {
            return button_state::on;
        } else {
            return button_state::off;
        }
    }

    void activate(abstract_button_widget& sender) noexcept override
    {
        value = *on_value;
    }
    /// @endprivatesection
private:
    typename decltype(value)::callback_token _value_cbt;
    typename decltype(on_value)::callback_token _on_value_cbt;
};

/** A default toggle button delegate.
 *
 * The default toggle button delegate manages the state of a button widget using
 * observer values.
 *
 * @ingroup widget_delegates
 * @tparam T The type of the observer value.
 */
template<typename T>
class default_toggle_button_delegate : public button_delegate {
public:
    using value_type = T;
    static constexpr bool can_make_defaults =
        std::is_same_v<value_type, bool> or std::is_integral_v<value_type> or std::is_enum_v<value_type>;

    observer<value_type> value;
    observer<value_type> on_value;
    observer<value_type> off_value;

    /** Construct a delegate.
     *
     * @param value A value or observer-value used as a representation of the state.
     * @param on_value The value or observer-value that mean 'on'.
     * @param off_value The value or observer-value that mean 'off'.
     */
    default_toggle_button_delegate(
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
    default_toggle_button_delegate(
        forward_of<observer<value_type>> auto&& value,
        forward_of<observer<value_type>> auto&& on_value) noexcept requires can_make_defaults :
        default_toggle_button_delegate(hi_forward(value), hi_forward(on_value), value_type{})
    {
    }

    /** Construct a delegate.
     *
     * @param value A value or observer-value used as a representation of the state.
     */
    default_toggle_button_delegate(forward_of<observer<value_type>> auto&& value) noexcept requires can_make_defaults :
        default_toggle_button_delegate(hi_forward(value), value_type{1}, value_type{})
    {
    }

    /// @privatesection
    [[nodiscard]] button_state state(abstract_button_widget const& sender) const noexcept override
    {
        if (*value == *on_value) {
            return button_state::on;
        } else if (*value == *off_value) {
            return button_state::off;
        } else {
            return button_state::other;
        }
    }

    void activate(abstract_button_widget& sender) noexcept override
    {
        if (*value == *off_value) {
            value = *on_value;
        } else {
            value = *off_value;
        }
    }
    /// @endprivatesection
private:
    typename decltype(value)::callback_token _value_cbt;
    typename decltype(on_value)::callback_token _on_value_cbt;
    typename decltype(off_value)::callback_token _off_value_cbt;
};

/** Make a shared pointer to a radio-button delegate.
 *
 * @ingroup widget_delegates
 * @see default_radio_button_delegate
 * @param value A value or observer-value used as a representation of the state.
 * @param on_value The value or observer-value that mean 'on'.
 * @return A shared_ptr to a button delegate.
 */
[[nodiscard]] std::shared_ptr<button_delegate>
make_default_radio_button_delegate(auto&& value, auto&& on_value) noexcept requires requires
{
    default_radio_button_delegate<observer_decay_t<decltype(value)>>{hi_forward(value), hi_forward(on_value)};
}
{
    return std::make_shared<default_radio_button_delegate<observer_decay_t<decltype(value)>>>(
        hi_forward(value), hi_forward(on_value));
}

/** Make a shared pointer to a toggle-button delegate.
 *
 * @ingroup widget_delegates
 * @see default_toggle_button_delegate
 * @param value A value or observer-value used as a representation of the state.
 * @param args an optional on-value followed by an optional off-value.
 * @return A shared_ptr to a button delegate.
 */
[[nodiscard]] std::shared_ptr<button_delegate>
make_default_toggle_button_delegate(auto&& value, auto&&...args) noexcept requires requires
{
    default_toggle_button_delegate<observer_decay_t<decltype(value)>>{hi_forward(value), hi_forward(args)...};
}
{
    return std::make_shared<default_toggle_button_delegate<observer_decay_t<decltype(value)>>>(
        hi_forward(value), hi_forward(args)...);
}

}} // namespace hi::v1
