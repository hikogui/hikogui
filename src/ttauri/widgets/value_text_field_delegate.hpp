// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "text_field_delegate.hpp"
#include "../observable.hpp"
#include "../label.hpp"
#include "../charconv.hpp"
#include <type_traits>
#include <memory>
#include <vector>
#include <concepts>

namespace tt {

template<typename T>
class value_text_field_delegate;

template<std::integral T>
class value_text_field_delegate<T> : public text_field_delegate {
public:
    using value_type = T;

    observable<value_type> value;

    template<typename Value>
    value_text_field_delegate(Value &&value) noexcept : value(std::forward<Value>(value))
    {
    }

    callback_ptr_type subscribe(text_field_widget &sender, callback_ptr_type const &callback_ptr) noexcept override
    {
        value.subscribe(callback_ptr);
        return callback_ptr;
    }

    void unsubscribe(text_field_widget &sender, callback_ptr_type const &callback_ptr) noexcept override
    {
        value.unsubscribe(callback_ptr);
    }

    std::optional<label> validate(text_field_widget &sender, std::string_view text) noexcept override
    {
        try {
            [[maybe_unused]] auto dummy = from_string<value_type>(text, 10);
        } catch (parse_error const &) {
            return {l10n{"Invalid integer"}};
        }

        return {};
    }

    std::string text(text_field_widget &sender) noexcept override
    {
        return to_string(*value);
    }

    void set_text(text_field_widget &sender, std::string_view text) noexcept override
    {
        try {
            value = from_string<value_type>(text, 10);
        } catch (std::exception const &) {
            // Ignore the error, don't modify the value.
            return;
        }
    }
};

template<typename Value>
std::shared_ptr<text_field_delegate> make_value_text_field_delegate(Value &&value) noexcept
{
    if constexpr (is_observable_v<std::remove_cvref_t<Value>>) {
        using value_type = typename std::remove_cvref_t<Value>::value_type;

        return std::make_shared<value_text_field_delegate<value_type>>(std::forward<Value>(value));

    } else {
        using value_type = std::remove_cvref_t<Value>;

        return std::make_shared<value_text_field_delegate<value_type>>(std::forward<Value>(value));
    }
}

} // namespace tt