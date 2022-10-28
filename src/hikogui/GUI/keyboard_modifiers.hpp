// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../utility.hpp"
#include "../exception.hpp"
#include "../strings.hpp"
#include "../cast.hpp"
#include <cstdint>

namespace hi::inline v1 {

/** Key modification keys pressed at the same time as another key.
 *
 * The Fn key is not always available on larger keyboards and is often under full
 * control of the keyboard, therefor it is not in the list of keyboard modifiers here.
 */
enum class keyboard_modifiers : uint8_t {
    none = 0x00,
    shift = 0x01, ///< The shift key is being held.
    control = 0x02, ///< The control key is being held.
    alt = 0x04, ///< The alt-key, option-key or meta-key is being held.
    super = 0x08, ///< The windows-key, key-key or super-key is being held.
};

[[nodiscard]] constexpr keyboard_modifiers operator|(keyboard_modifiers const& lhs, keyboard_modifiers const& rhs) noexcept
{
    return static_cast<keyboard_modifiers>(to_underlying(lhs) | to_underlying(rhs));
}

[[nodiscard]] constexpr keyboard_modifiers operator&(keyboard_modifiers const& lhs, keyboard_modifiers const& rhs) noexcept
{
    return static_cast<keyboard_modifiers>(to_underlying(lhs) & to_underlying(rhs));
}

constexpr keyboard_modifiers& operator|=(keyboard_modifiers& lhs, keyboard_modifiers const& rhs) noexcept
{
    return lhs = lhs | rhs;
}

[[nodiscard]] constexpr bool to_bool(keyboard_modifiers const& rhs) noexcept
{
    return to_bool(to_underlying(rhs));
}

bool operator>=(keyboard_modifiers const& lhs, keyboard_modifiers const& rhs) = delete;

/** Parse a key-binding modifier name.
 * @param s The modifier name, with or without the canonical trailing '+'
 */
inline keyboard_modifiers to_keyboard_modifiers(std::string_view s)
{
    if (ssize(s) == 0) {
        throw parse_error("Empty keyboard modifier");
    }

    // Remove the canonical trailing '+'.
    hilet s_lower = to_lower((s.back() == '+') ? s.substr(0, ssize(s) - 1) : s);

    if (s_lower == "shift") {
        return keyboard_modifiers::shift;
    } else if (s_lower == "control" || s_lower == "ctrl" || s_lower == "cntr") {
        return keyboard_modifiers::control;
    } else if (s_lower == "alt" || s_lower == "option" || s_lower == "meta") {
        return keyboard_modifiers::alt;
    } else if (s_lower == "windows" || s_lower == "win" || s_lower == "command" || s_lower == "cmd" || s_lower == "super") {
        return keyboard_modifiers::super;
    } else {
        throw parse_error(std::format("Unknown keyboard modifier '{}'", s));
    }
}

inline std::string to_string(keyboard_modifiers modifiers)
{
    auto r = std::string{};

    if (to_bool(modifiers & keyboard_modifiers::shift)) {
        r += "shift+";
    }
    if (to_bool(modifiers & keyboard_modifiers::control)) {
        r += "control+";
    }
    if (to_bool(modifiers & keyboard_modifiers::alt)) {
        r += "alt+";
    }
    if (to_bool(modifiers & keyboard_modifiers::super)) {
        r += "super+";
    }

    return r;
}

inline std::ostream& operator<<(std::ostream& lhs, keyboard_modifiers const& rhs)
{
    return lhs << to_string(rhs);
}

} // namespace hi::inline v1

template<>
struct std::hash<hi::keyboard_modifiers> {
    [[nodiscard]] std::size_t operator()(hi::keyboard_modifiers const& rhs) const noexcept
    {
        return std::hash<uint8_t>{}(static_cast<uint8_t>(rhs));
    }
};

template<typename CharT>
struct std::formatter<hi::keyboard_modifiers, CharT> : std::formatter<std::string_view, CharT> {
    auto format(hi::keyboard_modifiers const& t, auto& fc)
    {
        return std::formatter<std::string_view, CharT>::format(hi::to_string(t), fc);
    }
};
