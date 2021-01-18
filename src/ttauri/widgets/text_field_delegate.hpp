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
class text_field_widget;

template<typename T>
class text_field_delegate {
public:
    using value_type = T;

    /** The width of the text field in characters.
     *
     * @param self The widget controlled by this delegate.
     * @return The width of the text field box (exclusing prefix and suffix) in characters.
     */
    size_t text_width(text_field_widget &self) const noexcept
    {
        return 20;
    }

    /** The list of suggestions to show in the popup box.
     * 
     * @param self The widget controlled by this delegate.
     * @return A list of suffestion to show in the popup box.
     */
    std::vector<std::string> suggestions(text_field_widget &self) const noexcept
    {
        return {};
    }

    /** Convert a value to a string.
     *
     * @param self The widget controlled by this delegate.
     * @param value The original value before editing.
     * @return The string to be shown inside the text field box.
     */
    virtual std::string to_string(text_field_widget &self, value_type const &value) noexcept
    {
        // XXX Need to pass the current local to format.
        return fmt::format("{}", value);
    }

    /** Convert a string to a value.
     *
     * @param self The widget controlled by this delegate.
     * @param text The text string to convert to a value.
     * @param[out] error An localized error message to inform the user what is wrong.
     * @return value_type The value if the string conversion was successful. empty when the string
     *         conversion was NOT successful.
     */
    virtual std::optional<value_type> from_string(text_field_widget &self, std::string_view text, l10n &error) noexcept
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
