// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/required.hpp"
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

}

namespace std {

template<>
struct hash<TTauri::GUI::KeyboardModifiers> {
    [[nodiscard]] size_t operator() (TTauri::GUI::KeyboardModifiers const &rhs) const noexcept {
        return std::hash<uint8_t>{}(static_cast<uint8_t>(rhs));
    }
};

}