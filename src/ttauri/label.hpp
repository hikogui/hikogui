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

namespace tt {

/** A localized text + icon label.
 */
class label {
public:
    tt::icon icon;
    l10n text;

    label(tt::icon icon, l10n text) noexcept : icon(std::move(icon)), text(std::move(text)) {}

    label(l10n text) noexcept : icon(), text(std::move(text)) {}

    label(tt::icon icon) noexcept : icon(std::move(icon)), text() {}

    label() noexcept : icon(), text() {}

    label(label const &other) noexcept = default;
    label &operator=(label const &other) noexcept = default;
    label(label &&other) noexcept = default;
    label &operator=(label &&other) noexcept = default;

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

} // namespace tt

namespace std {

template<typename CharT>
struct std::formatter<tt::label, CharT> : std::formatter<std::string_view, CharT> {
    auto format(tt::label t, auto &fc)
    {
        return std::formatter<std::string_view, CharT>::format(to_string(t), fc);
    }
};

} // namespace std