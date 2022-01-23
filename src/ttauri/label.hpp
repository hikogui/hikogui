// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "required.hpp"
#include "strings.hpp"
#include "icon.hpp"
#include "l10n.hpp"
#include <string>
#include <type_traits>
#include <memory>

namespace tt::inline v1 {

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
    tt::icon icon;

    /** Localizable text.
     * The text in this field is not yet translated nor formatted.
     */
    l10n text;

    /** Construct a new label from an icon and text.
     * @param icon The icon.
     * @param text The text.
     */
    label(tt::icon icon, l10n text) noexcept : icon(std::move(icon)), text(std::move(text)) {}

    /** Construct a new label from text.
     * @param text The text.
     */
    label(l10n text) noexcept : icon(), text(std::move(text)) {}

    /** Construct a new label from an icon.
     * @param icon The icon.
     */
    label(tt::icon icon) noexcept : icon(std::move(icon)), text() {}

    /** Construct a empty label.
     */
    constexpr label() noexcept : icon(), text() {}

    label(label const &other) noexcept = default;
    label &operator=(label const &other) noexcept = default;
    label(label &&other) noexcept = default;
    label &operator=(label &&other) noexcept = default;

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
    [[nodiscard]] friend bool operator==(label const &lhs, label const &rhs) noexcept
    {
        return lhs.icon == rhs.icon and lhs.text == rhs.text;
    }

    [[nodiscard]] friend std::string to_string(label const &rhs) noexcept
    {
        return rhs.text();
    }

    friend std::ostream &operator<<(std::ostream &lhs, label const &rhs)
    {
        return lhs << to_string(rhs);
    }

private:
};

} // namespace tt::inline v1

template<typename CharT>
struct std::formatter<tt::label, CharT> : std::formatter<std::string_view, CharT> {
    auto format(tt::label const &t, auto &fc)
    {
        return std::formatter<std::string_view, CharT>::format(to_string(t), fc);
    }
};
