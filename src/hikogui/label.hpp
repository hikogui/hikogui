// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file label.hpp Functionality for labels, text and icons.
 */

#pragma once

#include "utility.hpp"
#include "strings.hpp"
#include "type_traits.hpp"
#include "fixed_string.hpp"
#include "i18n/translate.hpp"
#include "unicode/gstring.hpp"
#include "pixel_map.hpp"
#include "rapid/sfloat_rgba16.hpp"
#include "text/glyph_ids.hpp"
#include "text/elusive_icon.hpp"
#include "text/hikogui_icon.hpp"
#include <string>
#include <type_traits>
#include <memory>
#include <variant>

namespace hi::inline v1 {

/** A variant of text.
 *
 * May be:
 *  - `std::monostate`
 *  - `std::string`
 *  - `hi::gstring`
 *  - `hi::translate` or `hi::tr`
 */
class text : public std::variant<std::monostate, std::string, gstring, translate> {
    using std::variant<std::monostate, std::string, gstring, translate>::variant;

    /** Check if text contains a string.
     *
     * @note `to_bool()` returns true on an zero length string.
     */
    [[nodiscard]] constexpr friend bool to_bool(text const& rhs) noexcept
    {
        return not std::holds_alternative<std::monostate>(rhs);
    }

    /** Convert the text into a std::string.
     */
    [[nodiscard]] constexpr friend std::string to_string(hi::text const& rhs) noexcept
    {
        // clang-format off
        return std::visit(
            overloaded{
                [](std::monostate const &) { return std::string{}; },
                [](std::string const &x) { return x; },
                [](gstring const &x) { return hi::to_string(x); },
                [](translate const &x) { return x(); }
            },
            rhs);
        // clang-format on
    }

    /** Convert the text into a gstring.
     */
    [[nodiscard]] constexpr friend gstring to_gstring(hi::text const& rhs) noexcept
    {
        // clang-format off
        return std::visit(
            overloaded{
                [](std::monostate const &) { return gstring{}; },
                [](std::string const &x) { return to_gstring(std::string_view{x}); },
                [](gstring const &x) { return x; },
                [](translate const &x) { return to_gstring(std::string_view{x()}); }
            },
            rhs);
        // clang-format on
    }
};

/** A variant of icon.
 *
 * May be:
 *  - `std::monostate`
 *  - `hi::elusive_icon`
 *  - `hi::hikogui_icon`
 *  - `hi::glyph_ids`
 *  - `hi::pixel_map<hi::sfloat_rgba16>`
 */
class icon : public std::variant<std::monostate, elusive_icon, hikogui_icon, glyph_ids, pixel_map<sfloat_rgba16>>
{
    using std::variant<std::monostate, elusive_icon, hikogui_icon, glyph_ids, pixel_map<sfloat_rgba16>>::variant;

    /** Check if icon contains an image.
     */
    [[nodiscard]] constexpr friend bool to_bool(icon const& rhs) noexcept
    {
        return not std::holds_alternative<std::monostate>(rhs);
    }
};

} // namespace hi::inline v1

template<typename CharT>
struct std::formatter<hi::text, CharT> : std::formatter<std::string_view, CharT> {
    auto format(hi::text const& t, auto& fc)
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
struct std::formatter<hi::label, CharT> : std::formatter<hi::text, CharT> {
    auto format(hi::label const& t, auto& fc)
    {
        return std::formatter<hi::text, CharT>::format(t.text, fc);
    }
};
