// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file label.hpp Functionality for labels, text and icons.
 */

#pragma once

#include "utility/module.hpp"
#include "strings.hpp"
#include "i18n/translate.hpp"
#include "unicode/gstring.hpp"
#include "image/module.hpp"
#include "font/module.hpp"
#include <string>
#include <type_traits>
#include <memory>
#include <variant>

namespace hi::inline v1 {

/** A variant of text.
 *
 * May be:
 *  - `std::monostate`
 *  - `hi::text`
 *  - `hi::translate` or `hi::tr`
 */
class text_variant : public std::variant<std::monostate, hi::text, hi::translate> {
    using std::variant<std::monostate, hi::text, hi::translate>::variant;

    /** Check if text contains a string.
     *
     * @note `to_bool()` returns true on an zero length string.
     */
    [[nodiscard]] constexpr friend bool to_bool(text_variant const& rhs) noexcept
    {
        return not std::holds_alternative<std::monostate>(rhs);
    }

    /** Convert the text into a gstring.
     */
    [[nodiscard]] constexpr friend text to_text(hi::text_variant const& rhs) noexcept
    {
        // clang-format off
        return std::visit(
            overloaded{
                [](std::monostate const &) { return text{}; },
                [](hi::text const &x) { return x; },
                [](hi::translate const &x) { return x(); }
            },
            rhs);
        // clang-format on
    }

    [[nodiscard]] constexpr friend gstring to_gstring(hi::text_variant const& rhs) noexcept
    {
        return to_gstring(to_text(rhs));
    }

    [[nodiscard]] constexpr friend std::string to_string(hi::text_variant const& rhs) noexcept
    {
        return to_string(to_text(rhs));
    }

    [[nodiscard]] constexpr friend std::wstring to_wstring(hi::text_variant const& rhs) noexcept
    {
        return to_wstring(to_string(rhs));
    }
};

/** A variant of icon.
 *
 * May be:
 *  - `std::monostate`
 *  - `hi::elusive_icon`
 *  - `hi::hikogui_icon`
 *  - `hi::pixmap<hi::sfloat_rgba16>`
 */
class icon : public std::variant<std::monostate, elusive_icon, hikogui_icon, font_book::font_glyphs_type, pixmap<sfloat_rgba16>> {
    using std::variant<std::monostate, elusive_icon, hikogui_icon, font_book::font_glyphs_type, pixmap<sfloat_rgba16>>::variant;

    /** Check if icon contains an image.
     */
    [[nodiscard]] constexpr friend bool to_bool(icon const& rhs) noexcept
    {
        return not std::holds_alternative<std::monostate>(rhs);
    }
};

} // namespace hi::inline v1

template<typename CharT>
struct std::formatter<hi::text_variant, CharT> : std::formatter<std::string_view, CharT> {
    auto format(hi::text_variant const& t, auto& fc)
    {
        return std::formatter<std::string_view, CharT>::format(to_string(t), fc);
    }
};

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
    hi::text_variant text;

    /** Construct a new label from an icon and text.
     * @param icon The icon.
     * @param text The text.
     */
    constexpr label(std::convertible_to<hi::icon> auto&& icon, std::convertible_to<hi::text_variant> auto&& text) noexcept :
        icon(hi_forward(icon)), text(hi_forward(text))
    {
    }

    /** Construct a new label from text.
     * @param text The text.
     */
    constexpr label(std::convertible_to<hi::text_variant> auto&& text) noexcept : icon(), text(hi_forward(text)) {}

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
        return not(to_bool(icon) or to_bool(text));
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
    template<fixed_string>
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
struct std::formatter<hi::label, CharT> : std::formatter<hi::text_variant, CharT> {
    auto format(hi::label const& t, auto& fc)
    {
        return std::formatter<hi::text_variant, CharT>::format(t.text, fc);
    }
};
