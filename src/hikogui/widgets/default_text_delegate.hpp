// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "text_delegate.hpp"
#include "../observer.hpp"
#include <type_traits>
#include <memory>

namespace hi::inline v1 {

/** A default text delegate.
 *
 * @tparam T The type of the observer value.
 */
template<typename T>
class default_text_delegate : public text_delegate {
public:
    using value_type = T;

    observer<value_type> value;

    /** Construct a delegate.
     *
     * @param value A value or observable-value used as a representation of the state.
     */
    default_text_delegate(auto&& value) noexcept : value(hi_forward(value))
    {
        // clang-format off
        _value_cbt = this->value.subscribe(callback_flags::local,[&](auto...){ this->_notifier(); });
        // clang-format on
    }

    /// @privatesection
    [[nodiscard]] gstring read(text_widget const &sender) const noexcept override
    {
        if constexpr (std::is_same_v<value_type, std::string>) {
            return to_gstring(*value);

        } else if constexpr (std::is_same_v<value_type, translate>) {
            return to_gstring(value());

        } else if constexpr (std::is_same_v<value_type, gstring>) {
            return *value;

        } else if constexpr (std::is_same_v<value_type, text>) {
            return to_gstring(*value);

        } else {
            hi_not_implemented();
        }
    }

    void write(text_widget const& sender, gstring const &text) const noexcept override
    {
        if constexpr (std::is_same_v<value_type, std::string>) {
            *value.copy() = to_string(text);

        } else if constexpr (std::is_same_v<value_type, gstring>) {
            *value.copy() = text;

        } else if constexpr (std::is_same_v<value_type, text>) {
            auto proxy = value.copy();
            auto *ptr = std::addressof(*proxy);

            if (auto *string_ptr = get_if<std::string>(ptr)) {
                *string_ptr = to_string(text);
            } else if (auto *gstring_ptr = get_if<gstring>(ptr)) {
                *string_ptr = text;
            } else {
                hi_not_implemented();
            }

        } else {
            hi_not_implemented();
        }
    }
    /// @endprivatesection
private:
    typename decltype(value)::token_type _value_cbt;
};

template<typename Value>
default_text_delegate(Value&&) -> default_text_delegate<observer_argument_t<Value>>;

std::unique_ptr<text_delegate> make_unique_default_text_delegate(Value&& value) noexcept
{
    using value_type = observer_argument_t<Value>;
    return std::make_unique<default_text_delegate<value_type>>(std::forward<Value>(value));
}

} // namespace hi::inline v1
