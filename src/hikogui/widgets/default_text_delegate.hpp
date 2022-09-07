// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "text_delegate.hpp"
#include "../observer.hpp"
#include "../text.hpp"
#include "../type_traits.hpp"
#include "../concepts.hpp"
#include "../unicode/gstring.hpp"
#include "../i18n/translate.hpp"
#include <type_traits>
#include <memory>
#include <string>

namespace hi::inline v1 {

/** A default text delegate.
 *
 * @tparam T The type of the observer value.
 */
template<typename T>
class default_text_delegate;

template<>
class default_text_delegate<std::string> : public text_delegate {
public:
    using value_type = std::string;

    observer<value_type> value;

    /** Construct a delegate.
     *
     * @param value A value or observable-value used as a representation of the state.
     */
    explicit default_text_delegate(forward_of<observer<value_type>> auto&& value) noexcept : value(hi_forward(value))
    {
        _value_cbt = this->value.subscribe(callback_flags::synchronous, [&](auto...) {
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
    typename decltype(value)::token_type _value_cbt;
};

template<>
class default_text_delegate<gstring> : public text_delegate {
public:
    using value_type = gstring;

    observer<value_type> value;

    /** Construct a delegate.
     *
     * @param value A value or observable-value used as a representation of the state.
     */
    explicit default_text_delegate(forward_of<observer<value_type>> auto&& value) noexcept : value(hi_forward(value))
    {
        _value_cbt = this->value.subscribe(callback_flags::synchronous, [&](auto...) {
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
    typename decltype(value)::token_type _value_cbt;
};

template<>
class default_text_delegate<translate> : public text_delegate {
public:
    using value_type = translate;

    observer<value_type> value;

    /** Construct a delegate.
     *
     * @param value A value or observable-value used as a representation of the state.
     */
    explicit default_text_delegate(forward_of<observer<value_type>> auto&& value) noexcept : value(hi_forward(value))
    {
        _value_cbt = this->value.subscribe(callback_flags::synchronous, [&](auto...) {
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
    typename decltype(value)::token_type _value_cbt;
};

template<>
class default_text_delegate<text> : public text_delegate {
public:
    using value_type = text;

    observer<value_type> value;

    /** Construct a delegate.
     *
     * @param value A value or observable-value used as a representation of the state.
     */
    explicit default_text_delegate(forward_of<observer<value_type>> auto&& value) noexcept : value(hi_forward(value))
    {
        _value_cbt = this->value.subscribe(callback_flags::synchronous, [&](auto...) {
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
    typename decltype(value)::token_type _value_cbt;
};

std::shared_ptr<text_delegate> make_default_text_delegate(auto&& value) noexcept requires requires
{
    default_text_delegate<observer_argument_t<decltype(value)>>{hi_forward(value)};
}
{
    using value_type = observer_argument_t<decltype(value)>;
    return std::make_shared<default_text_delegate<value_type>>(hi_forward(value));
}

} // namespace hi::inline v1
