// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "required.hpp"
#include "strings.hpp"
#include "icon.hpp"
#include "text.hpp"
#include "type_traits.hpp"
#include "fixed_string.hpp"
#include "i18n/translate.hpp"
#include <string>
#include <type_traits>
#include <memory>

namespace hi::inline v1 {

/** A label consisting of localizable text and an icon.
 *
 * A label is used for user-visible information. The label is used as
 * information displayed by the `label_widget`.
 *
 * The audio subsystem will use labels to return user-visible information such
 * as the name of audio device end-points or surround sound speaker
 * configuration which in both cases includes icons and text that needs to be
 * translated.
 */
class label {
public:
    /** The icon.
     */
    hi::icon icon;

    /** Localizable text.
     * The text in this field is not yet translated nor formatted.
     */
    hi::text text;

    /** Construct a new label from an icon and text.
     * @param icon The icon.
     * @param text The text.
     */
    constexpr label(std::convertible_to<hi::icon> auto&& icon, std::convertible_to<hi::text> auto&& text) noexcept :
        icon(hi_forward(icon)), text(hi_forward(text))
    {
    }

    /** Construct a new label from text.
     * @param text The text.
     */
    constexpr label(std::convertible_to<hi::text> auto&& text) noexcept : icon(), text(hi_forward(text)) {}

    /** Construct a new label from an icon.
     * @param icon The icon.
     */
    constexpr label(std::convertible_to<hi::icon> auto&& icon) noexcept : icon(hi_forward(icon)), text() {}

    /** Construct a empty label.
     */
    constexpr label() noexcept : icon(), text() {}

    constexpr label(label const& other) noexcept = default;
    constexpr label& operator=(label const& other) noexcept = default;
    constexpr label(label&& other) noexcept = default;
    constexpr label& operator=(label&& other) noexcept = default;

    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return icon.empty() and text.empty();
    }

    constexpr explicit operator bool() const noexcept
    {
        return not empty();
    }

    /** Compare if both labels are equal.
     * @param lhs A label.
     * @param rhs A label.
     * @return True is the text and icon of both labels are equal.
     */
    [[nodiscard]] constexpr friend bool operator==(label const& lhs, label const& rhs) noexcept = default;
};

template<>
struct selector<label> {
    template<basic_fixed_string>
    [[nodiscard]] auto& get(label&) const noexcept;

    template<>
    [[nodiscard]] auto& get<"text">(label& rhs) const noexcept
    {
        return rhs.text;
    }

    template<>
    [[nodiscard]] auto& get<"icon">(label& rhs) const noexcept
    {
        return rhs.icon;
    }
};

} // namespace hi::inline v1

template<typename CharT>
struct std::formatter<hi::label, CharT> : std::formatter<hi::text, CharT> {
    auto format(hi::label const& t, auto& fc)
    {
        return std::formatter<hi::text, CharT>::format(t.text, fc);
    }
};
