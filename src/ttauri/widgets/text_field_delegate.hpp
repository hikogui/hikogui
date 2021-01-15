// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include <string>
#include <string_view>
#include <optional>
#include <type_traits>
#include <concepts>
#include <memory>
#include "../l10n.hpp"
#include "../concepts.hpp"

namespace tt {

template<typename T>
class text_field_delegate {
public:
    using value_type = T;

    /** The width of the text field in characters.
     */
    size_t text_width() const noexcept
    {
        return 20;
    }

    /** Convert a value to a string.
     */
    virtual std::string to_string(value_type const &value) noexcept
    {
        // XXX Need to pass the current local to format.
        return fmt::format("{}", value);
    }

    /** Convert a string to a value.
     * @param text The text string to convert to a value.
     * @param[out] error An localized error message to inform the user what is wrong.
     * @return value_type The value if the string conversion was successful. empty when the string
     *         conversion was NOT successful.
     */
    virtual std::optional<value_type> from_string(std::string_view text, l10n &error) noexcept
    {
        try {
            error = {};
            return tt::from_string<value_type>(text);
        } catch (parse_error) {
            error = l10n("Invalid character entered.");
            return {};
        }
    }
};

template<typename T>
inline std::shared_ptr<text_field_delegate<T>> text_field_delegate_default() noexcept
{
    static std::shared_ptr<text_field_delegate<T>> delegate = std::make_shared<text_field_delegate<T>>();
    return delegate;
}

} // namespace tt