// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file label.hpp Functionality for labels, text and icons.
 */

#pragma once

#include "../utility/utility.hpp"
#include "../unicode/unicode.hpp"
#include "../image/image.hpp"
#include "../font/font.hpp"
#include "txt.hpp"
#include "../macros.hpp"
#include <string>
#include <type_traits>
#include <memory>
#include <variant>

hi_export_module(hikogui.l10n.label);


hi_export namespace hi::inline v1 {

/** A variant of icon.
 *
 * May be:
 *  - `std::monostate`
 *  - `hi::elusive_icon`
 *  - `hi::hikogui_icon`
 *  - `hi::font_book::font_glyph_type`
 *  - `hi::pixmap<hi::sfloat_rgba16>`
 */
class icon : public std::variant<std::monostate, elusive_icon, hikogui_icon, font_book::font_glyph_type, pixmap<sfloat_rgba16>>
{
    using std::variant<std::monostate, elusive_icon, hikogui_icon, font_book::font_glyph_type, pixmap<sfloat_rgba16>>::variant;

    /** Check if icon contains an image.
     */
    [[nodiscard]] constexpr friend bool to_bool(icon const& rhs) noexcept
    {
        return not std::holds_alternative<std::monostate>(rhs);
    }
};

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
    txt text;

    /** Construct a new label from an icon and text.
     * @param icon The icon.
     * @param text The text.
     */
    template<std::convertible_to<hi::icon> Icon, std::convertible_to<txt> Txt>
    constexpr label(Icon && icon, Txt && text) noexcept :
        icon(std::forward<Icon>(icon)), text(std::forward<Txt>(text))
    {
    }

    /** Construct a new label from text.
     * @param text The text.
     */
    template<std::convertible_to<txt> Txt>
    constexpr label(Txt && text) noexcept : icon(), text(std::forward<Txt>(text)) {}

    /** Construct a new label from an icon.
     * @param icon The icon.
     */
    template<std::convertible_to<hi::icon> Icon>
    constexpr label(Icon&& icon) noexcept : icon(std::forward<Icon>(icon)), text() {}

    /** Construct a empty label.
     */
    constexpr label() noexcept : icon(), text() {}

    label(label const& other) noexcept = default;
    label& operator=(label const& other) noexcept = default;
    label(label&& other) noexcept = default;
    label& operator=(label&& other) noexcept = default;

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
    [[nodiscard]] constexpr friend bool operator==(label const&, label const&) noexcept = default;
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

// XXX #617 MSVC bug does not handle partial specialization in modules.
hi_export template<>
struct std::formatter<hi::label, char> : std::formatter<std::string_view, char> {
    auto format(hi::label const& t, auto& fc) const
    {
        return std::formatter<std::string_view, char>::format(std::string{t.text}, fc);
    }
};
