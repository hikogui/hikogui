// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/required.hpp"
#include "TTauri/Foundation/exceptions.hpp"
#include "TTauri/Foundation/strings.hpp"
#include <cstdint>

namespace TTauri::GUI {

/** Key modification keys pressed at the same time as another key.
*
* The Fn key is not always available on larger keyboards and is often under full
* control of the keyboard, therefor it is not in the list of keyboard modifiers here.
*/
enum class KeyboardModifiers : uint8_t {
    None = 0x00,
    Shift = 0x01,   ///< The shift key is being held.
    Control = 0x02, ///< The control key is being held.
    Alt = 0x04,     ///< The alt-key, option-key or meta-key is being held.
    Super = 0x08,   ///< The windows-key, key-key or super-key is being held.
};

[[nodiscard]] constexpr KeyboardModifiers operator|(KeyboardModifiers lhs, KeyboardModifiers rhs) noexcept
{
    return static_cast<KeyboardModifiers>(static_cast<uint8_t>(lhs) | static_cast<uint8_t>(rhs));
}

constexpr KeyboardModifiers &operator|=(KeyboardModifiers &lhs, KeyboardModifiers rhs) noexcept
{
    lhs = lhs | rhs;
    return lhs;
}

[[nodiscard]] constexpr bool operator>=(KeyboardModifiers lhs, KeyboardModifiers rhs) noexcept {
    let lhs_ = static_cast<uint8_t>(lhs);
    let rhs_ = static_cast<uint8_t>(rhs);
    return (lhs_ & rhs_) == rhs_;
}

/** Parse a key-binding modifier name.
 * @param s The modifier name, with or without the canonical trailing '+'
 */
inline KeyboardModifiers to_KeyboardModifiers(std::string_view s)
{
    if (ssize(s) == 0) {
        TTAURI_THROW(parse_error("Empty keyboard modifier"));       
    }

    // Remove the canonical trailing '+'.
    let s_lower = to_lower(
        (s.back() == '+') ? s.substr(0, ssize(s) - 1) : s
    );

    if (s_lower == "shift") {
        return KeyboardModifiers::Shift;
    } else if (s_lower == "control" || s_lower == "ctrl" || s_lower == "cntr") {
        return KeyboardModifiers::Control;
    } else if (s_lower == "alt" || s_lower == "option" || s_lower == "meta") {
        return KeyboardModifiers::Alt;
    } else if (
        s_lower == "windows" || s_lower == "win" ||
        s_lower == "command" || s_lower == "cmd" ||
        s_lower == "super"
    ) {
        return KeyboardModifiers::Super;
    } else {
        TTAURI_THROW(parse_error("Unknown keyboard modifier '{}'", s));
    }
}

inline std::string to_string(KeyboardModifiers modifiers)
{
    auto r = std::string{};

    if (modifiers >= KeyboardModifiers::Shift) {
        r += "shift+";
    }
    if (modifiers >= KeyboardModifiers::Control) {
        r += "control+";
    }
    if (modifiers >= KeyboardModifiers::Alt) {
        r += "alt+";
    }
    if (modifiers >= KeyboardModifiers::Super) {
        r += "super+";
    }

    return r;
}

inline std::ostream &operator<<(std::ostream &lhs, KeyboardModifiers const &rhs)
{
    return lhs << to_string(rhs);
}

}

namespace std {

template<>
struct hash<TTauri::GUI::KeyboardModifiers> {
    [[nodiscard]] size_t operator() (TTauri::GUI::KeyboardModifiers const &rhs) const noexcept {
        return std::hash<uint8_t>{}(static_cast<uint8_t>(rhs));
    }
};

}