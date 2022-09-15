// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/text_delegate.hpp Defines delegate_delegate and some default text delegates.
 * @ingroup widget_delegates
 */

#pragma once

#include "../i18n/translate.hpp"
#include "../unicode/gstring.hpp"
#include "../label.hpp"
#include <string>
#include <memory>
#include <functional>

namespace hi { inline namespace v1 {
class text_widget;

/** A delegate that controls the state of a text_widget.
 *
 * @ingroup widget_delegates
 */
class text_delegate {
public:
    using notifier_type = notifier<>;
    using callback_token = notifier_type::callback_token;
    using callback_proto = notifier_type::callback_proto;

    virtual ~text_delegate() = default;

    virtual void init(text_widget& sender) noexcept {}
    virtual void deinit(text_widget& sender) noexcept {}

    /** Read text as a string of graphemes.
     */
    [[nodiscard]] virtual gstring read(text_widget& sender) noexcept = 0;

    /** Write text from a string of graphemes.
     */
    virtual void write(text_widget& sender, gstring const& text) noexcept = 0;

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

/** A default text delegate.
 *
 * @ingroup widget_delegates
 * @tparam T The type of the observer value.
 */
template<typename T>
class default_text_delegate;

/** A default text delegate specialization for `std::string`.
 *
 * @ingroup widget_delegates
 */
template<>
class default_text_delegate<std::string> : public text_delegate {
public:
    using value_type = std::string;

    observer<value_type> value;

    /** Construct a delegate.
     *
     * @param value A value or observer-value used as a representation of the state.
     */
    explicit default_text_delegate(forward_of<observer<value_type>> auto&& value) noexcept : value(hi_forward(value))
    {
        _value_cbt = this->value.subscribe([&](auto...) {
            this->_notifier();
        });
    }

    [[nodiscard]] gstring read(text_widget& sender) noexcept override
    {
        return to_gstring(*value);
    }

    void write(text_widget& sender, gstring const& text) noexcept override
    {
        *value.copy() = to_string(text);
    }

private:
    typename decltype(value)::callback_token _value_cbt;
};

/** A default text delegate specialization for `gstring`.
 *
 * @ingroup widget_delegates
 */
template<>
class default_text_delegate<gstring> : public text_delegate {
public:
    using value_type = gstring;

    observer<value_type> value;

    /** Construct a delegate.
     *
     * @param value A value or observer-value used as a representation of the state.
     */
    explicit default_text_delegate(forward_of<observer<value_type>> auto&& value) noexcept : value(hi_forward(value))
    {
        _value_cbt = this->value.subscribe([&](auto...) {
            this->_notifier();
        });
    }

    [[nodiscard]] gstring read(text_widget& sender) noexcept override
    {
        return *value;
    }

    void write(text_widget& sender, gstring const& text) noexcept override
    {
        *value.copy() = text;
    }

private:
    typename decltype(value)::callback_token _value_cbt;
};

/** A default text delegate specialization for `translate`.
 *
 * @ingroup widget_delegates
 */
template<>
class default_text_delegate<translate> : public text_delegate {
public:
    using value_type = translate;

    observer<value_type> value;

    /** Construct a delegate.
     *
     * @param value A value or observer-value used as a representation of the state.
     */
    explicit default_text_delegate(forward_of<observer<value_type>> auto&& value) noexcept : value(hi_forward(value))
    {
        _value_cbt = this->value.subscribe([&](auto...) {
            this->_notifier();
        });
    }

    [[nodiscard]] gstring read(text_widget& sender) noexcept override
    {
        return to_gstring(value.read()());
    }

    void write(text_widget& sender, gstring const& text) noexcept override
    {
        hi_no_default();
    }

private:
    typename decltype(value)::callback_token _value_cbt;
};

/** A default text delegate specialization for `text`.
 *
 * @ingroup widget_delegates
 */
template<>
class default_text_delegate<text> : public text_delegate {
public:
    using value_type = text;

    observer<value_type> value;

    /** Construct a delegate.
     *
     * @param value A value or observer-value used as a representation of the state.
     */
    explicit default_text_delegate(forward_of<observer<value_type>> auto&& value) noexcept : value(hi_forward(value))
    {
        _value_cbt = this->value.subscribe([&](auto...) {
            this->_notifier();
        });
    }

    [[nodiscard]] gstring read(text_widget& sender) noexcept override
    {
        return to_gstring(*value.read());
    }

    void write(text_widget& sender, gstring const& text) noexcept override
    {
        auto proxy = value.copy();
        auto *ptr = std::addressof(*proxy);

        if (auto *string_ptr = get_if<std::string>(ptr)) {
            *string_ptr = to_string(text);
        } else if (auto *gstring_ptr = get_if<gstring>(ptr)) {
            *gstring_ptr = text;
        } else {
            hi_not_implemented();
        }
    }

private:
    typename decltype(value)::callback_token _value_cbt;
};

/** Create a shared pointer to a default text delegate.
 *
 * @ingroup widget_delegates
 * @see default_text_delegate
 * @param value The observer value which represents the displayed text.
 * @return shared pointer to a text delegate
 */
std::shared_ptr<text_delegate> make_default_text_delegate(auto&& value) noexcept requires requires
{
    default_text_delegate<observer_decay_t<decltype(value)>>{hi_forward(value)};
}
{
    return std::make_shared<default_text_delegate<observer_decay_t<decltype(value)>>>(hi_forward(value));
}

}} // namespace hi::v1
