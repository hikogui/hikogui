// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../required.hpp"
#include "../exception.hpp"
#include "../strings.hpp"
#include <cstdint>

namespace tt {

/** Key modification keys pressed at the same time as another key.
 *
 * The Fn key is not always available on larger keyboards and is often under full
 * control of the keyboard, therefor it is not in the list of keyboard modifiers here.
 */
enum class keyboard_modifiers : uint8_t {
    None = 0x00,
    Shift = 0x01, ///< The shift key is being held.
    Control = 0x02, ///< The control key is being held.
    Alt = 0x04, ///< The alt-key, option-key or meta-key is being held.
    Super = 0x08, ///< The windows-key, key-key or super-key is being held.
};

[[nodiscard]] constexpr keyboard_modifiers operator|(keyboard_modifiers lhs, keyboard_modifiers rhs) noexcept
{
    return static_cast<keyboard_modifiers>(static_cast<uint8_t>(lhs) | static_cast<uint8_t>(rhs));
}

constexpr keyboard_modifiers &operator|=(keyboard_modifiers &lhs, keyboard_modifiers rhs) noexcept
{
    lhs = lhs | rhs;
    return lhs;
}

[[nodiscard]] constexpr bool operator>=(keyboard_modifiers lhs, keyboard_modifiers rhs) noexcept
{
    ttlet lhs_ = static_cast<uint8_t>(lhs);
    ttlet rhs_ = static_cast<uint8_t>(rhs);
    return (lhs_ & rhs_) == rhs_;
}

/** Parse a key-binding modifier name.
 * @param s The modifier name, with or without the canonical trailing '+'
 */
inline keyboard_modifiers to_keyboard_modifiers(std::string_view s)
{
    if (std::ssize(s) == 0) {
        throw parse_error("Empty keyboard modifier");
    }

    // Remove the canonical trailing '+'.
    ttlet s_lower = to_lower((s.back() == '+') ? s.substr(0, std::ssize(s) - 1) : s);

    if (s_lower == "shift") {
        return keyboard_modifiers::Shift;
    } else if (s_lower == "control" || s_lower == "ctrl" || s_lower == "cntr") {
        return keyboard_modifiers::Control;
    } else if (s_lower == "alt" || s_lower == "option" || s_lower == "meta") {
        return keyboard_modifiers::Alt;
    } else if (s_lower == "windows" || s_lower == "win" || s_lower == "command" || s_lower == "cmd" || s_lower == "super") {
        return keyboard_modifiers::Super;
    } else {
        throw parse_error("Unknown keyboard modifier '{}'", s);
    }
}

inline std::string to_string(keyboard_modifiers modifiers)
{
    auto r = std::string{};

    if (modifiers >= keyboard_modifiers::Shift) {
        r += "shift+";
    }
    if (modifiers >= keyboard_modifiers::Control) {
        r += "control+";
    }
    if (modifiers >= keyboard_modifiers::Alt) {
        r += "alt+";
    }
    if (modifiers >= keyboard_modifiers::Super) {
        r += "super+";
    }

    return r;
}

inline std::ostream &operator<<(std::ostream &lhs, keyboard_modifiers const &rhs)
{
    return lhs << to_string(rhs);
}

} // namespace tt

namespace std {

template<>
struct hash<tt::keyboard_modifiers> {
    [[nodiscard]] size_t operator()(tt::keyboard_modifiers const &rhs) const noexcept
    {
        return std::hash<uint8_t>{}(static_cast<uint8_t>(rhs));
    }
};

template<typename CharT>
struct std::formatter<tt::keyboard_modifiers, CharT> : std::formatter<std::string_view, CharT> {
    auto format(tt::keyboard_modifiers const &t, auto &fc)
    {
        return std::formatter<std::string_view, CharT>::format(tt::to_string(t), fc);
    }
};

} // namespace std
