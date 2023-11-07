// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/text_field_delegate.hpp Defines delegate_field_delegate and some default text field delegates.
 * @ingroup widget_delegates
 */

#pragma once

#include "../observer/observer.hpp"
#include "../utility/utility.hpp"
#include "../concurrency/concurrency.hpp"
#include "../dispatch/dispatch.hpp"
#include "../GUI/GUI.hpp"
#include "../macros.hpp"
#include "../l10n/l10n.hpp"
#include "../macros.hpp"
#include <string>
#include <string_view>
#include <optional>
#include <concepts>

hi_export_module(hikogui.widgets.text_field_delegate);

hi_export namespace hi { inline namespace v1 {

/** A delegate that controls the state of a text_field_widget.
 *
 * @ingroup widget_delegates
 */
class text_field_delegate {
public:
    using notifier_type = notifier<void()>;

    virtual ~text_field_delegate() = default;
    virtual void init(widget_intf const& sender) noexcept {}
    virtual void deinit(widget_intf const& sender) noexcept {}

    /** Validate the text field.
     *
     * @param sender The widget that called this function.
     * @param text The text entered by the user into the text field.
     * @return no-value when valid, or a label to display to the user when invalid.
     */
    virtual label validate(widget_intf const& sender, gstring const& text) noexcept
    {
        return {};
    }

    /** Get the text to show in the text field.
     * When the user is not editing the text the text-field will request what to show
     * using this function.
     *
     * @param sender The widget that called this function.
     * @return The text to show in the text field.
     */
    virtual gstring text(widget_intf const& sender) noexcept
    {
        return {};
    }

    /** Set the text as entered by the user.
     * When the user causes a text field to commit,
     * by pressing enter, tab, or clicking outside the field and when
     * the text was validated the widget will call this function to commit the
     * text with the delegate.
     *
     * @pre text Must have been validated as correct.
     * @param sender The widget that called this function.
     * @param text The text entered by the user.
     */
    virtual void set_text(widget_intf const& sender, gstring const& text) noexcept {}

    template<forward_of<void()> Func>
    callback<void()> subscribe(Func&& func, callback_flags flags = callback_flags::synchronous) noexcept
    {
        return _notifier.subscribe(std::forward<Func>(func), flags);
    }

protected:
    notifier_type _notifier;
};

/** A default text delegate.
 *
 * @ingroup widget_delegates
 * @tparam T The type of the observer value.
 */
template<typename T>
class default_text_field_delegate;

/** A default text delegate specialization for `std::integral<T>`.
 * This delegate makes it possible for a text-field to edit an integral value.
 * It will automatically validate and convert between the integral value and the text representation.
 *
 * @ingroup widget_delegates
 * @tparam T An integral type
 */
template<std::integral T>
class default_text_field_delegate<T> : public text_field_delegate {
public:
    using value_type = T;

    observer<value_type> value;

    default_text_field_delegate(forward_of<observer<value_type>> auto&& value) noexcept : value(hi_forward(value))
    {
        _value_cbt = this->value.subscribe([&](auto...) {
            this->_notifier();
        });
    }

    label validate(widget_intf const& sender, gstring const& text) noexcept override
    {
        try {
            [[maybe_unused]] auto dummy = from_string<value_type>(to_string(text), 10);
        } catch (parse_error const&) {
            return {txt("Invalid integer")};
        }

        return {};
    }

    gstring text(widget_intf const& sender) noexcept override
    {
        return to_gstring(to_string(*value));
    }

    void set_text(widget_intf const& sender, gstring const& text) noexcept override
    {
        try {
            value = from_string<value_type>(to_string(text), 10);
        } catch (std::exception const&) {
            // Ignore the error, don't modify the value.
            return;
        }
    }

private:
    callback<void(value_type)> _value_cbt;
};

/** A default text delegate specialization for `std::floating_point<T>`.
 * This delegate makes it possible for a text-field to edit an floating point value.
 * It will automatically validate and convert between the floating point value and the text representation.
 *
 * @ingroup widget_delegates
 * @tparam T An floating point type
 */
template<std::floating_point T>
class default_text_field_delegate<T> : public text_field_delegate {
public:
    using value_type = T;

    observer<value_type> value;

    default_text_field_delegate(forward_of<observer<value_type>> auto&& value) noexcept : value(hi_forward(value))
    {
        _value_cbt = this->value.subscribe([&](auto...) {
            this->_notifier();
        });
    }

    label validate(widget_intf const& sender, gstring const& text) noexcept override
    {
        try {
            [[maybe_unused]] auto dummy = from_string<value_type>(to_string(text));
        } catch (parse_error const&) {
            return {elusive_icon::WarningSign, txt("Invalid floating point number")};
        }

        return {};
    }

    gstring text(widget_intf const& sender) noexcept override
    {
        return to_gstring(to_string(*value));
    }

    void set_text(widget_intf const& sender, gstring const& text) noexcept override
    {
        try {
            value = from_string<value_type>(to_string(text));
        } catch (std::exception const&) {
            // Ignore the error, don't modify the value.
            return;
        }
    }

private:
    callback<void(value_type)> _value_cbt;
};

/** Create a shared pointer to a default text delegate.
 *
 * @ingroup widget_delegates
 * @see default_text_field_delegate
 * @param value The observer value which is editable by the text field widget.
 * @return shared pointer to a text field delegate
 */
[[nodiscard]] std::shared_ptr<text_field_delegate> make_default_text_field_delegate(auto&& value) noexcept
    requires requires { default_text_field_delegate<observer_decay_t<decltype(value)>>{hi_forward(value)}; }
{
    using value_type = observer_decay_t<decltype(value)>;
    return std::make_shared<default_text_field_delegate<value_type>>(hi_forward(value));
}

}} // namespace hi::v1
