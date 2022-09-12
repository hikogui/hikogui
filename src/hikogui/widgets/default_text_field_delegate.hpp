// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "text_field_delegate.hpp"
#include "../observer.hpp"
#include "../label.hpp"
#include "../charconv.hpp"
#include "../type_traits.hpp"
#include <type_traits>
#include <memory>
#include <vector>
#include <concepts>

namespace hi::inline v1 {
template<typename T>
class default_text_field_delegate;

template<std::integral T>
class default_text_field_delegate<T> : public text_field_delegate {
public:
    using value_type = T;

    observer<value_type> value;

    default_text_field_delegate(forward_of<observer<value_type>> auto&& value) noexcept : value(hi_forward(value))
    {
        _value_cbt = this->value.subscribe(callback_flags::synchronous, [&](auto...) {
            this->_notifier();
        });
    }

    std::optional<label> validate(text_field_widget& sender, std::string_view text) noexcept override
    {
        try {
            [[maybe_unused]] auto dummy = from_string<value_type>(text, 10);
        } catch (parse_error const&) {
            return {tr{"Invalid integer"}};
        }

        return {};
    }

    std::string text(text_field_widget& sender) noexcept override
    {
        return to_string(*value);
    }

    void set_text(text_field_widget& sender, std::string_view text) noexcept override
    {
        try {
            value = from_string<value_type>(text, 10);
        } catch (std::exception const&) {
            // Ignore the error, don't modify the value.
            return;
        }
    }

private:
    typename decltype(value)::token_type _value_cbt;
};

template<std::floating_point T>
class default_text_field_delegate<T> : public text_field_delegate {
public:
    using value_type = T;

    observer<value_type> value;

    default_text_field_delegate(forward_of<observer<value_type>> auto&& value) noexcept : value(hi_forward(value))
    {
        _value_cbt = this->value.subscribe(callback_flags::synchronous, [&](auto...) {
            this->_notifier();
        });
    }

    label validate(text_field_widget& sender, std::string_view text) noexcept override
    {
        try {
            [[maybe_unused]] auto dummy = from_string<value_type>(text);
        } catch (parse_error const&) {
            return {elusive_icon::WarningSign, tr{"Invalid floating point number"}};
        }

        return {};
    }

    std::string text(text_field_widget& sender) noexcept override
    {
        return to_string(*value);
    }

    void set_text(text_field_widget& sender, std::string_view text) noexcept override
    {
        try {
            value = from_string<value_type>(text);
        } catch (std::exception const&) {
            // Ignore the error, don't modify the value.
            return;
        }
    }

private:
    typename decltype(value)::token_type _value_cbt;
};

[[nodiscard]] std::shared_ptr<text_field_delegate>
make_default_text_field_delegate(auto&& value, auto&&...args) noexcept requires requires
{
    default_text_field_delegate<observer_argument_t<decltype(value)>>{hi_forward(value), hi_forward(args)...};
}
{
    using value_type = observer_argument_t<decltype(value)>;
    return std::make_shared<default_text_field_delegate<value_type>>(hi_forward(value), hi_forward(args)...);
}

} // namespace hi::inline v1
