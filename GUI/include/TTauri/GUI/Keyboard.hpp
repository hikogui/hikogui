// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/GUI/KeyboardKey.hpp"
#include "TTauri/Text/Grapheme.hpp"
#include "TTauri/Foundation/required.hpp"
#include "TTauri/Foundation/assert.hpp"
#include <utility>

namespace TTauri::GUI {

enum class KeyboardState : uint8_t {
    Idle = 0x00,
    CapsLock = 0x01,
    ScrollLock = 0x02,
    NumLock = 0x04,
};

[[nodiscard]] constexpr KeyboardState operator|(KeyboardState lhs, KeyboardState rhs) noexcept {
    return static_cast<KeyboardState>(static_cast<uint8_t>(lhs) | static_cast<uint8_t>(rhs));
}

constexpr KeyboardState &operator|=(KeyboardState &lhs, KeyboardState rhs) noexcept {
    lhs = lhs | rhs;
    return lhs;
}

[[nodiscard]] constexpr bool operator>=(KeyboardState lhs, KeyboardState rhs) noexcept {
    let lhs_ = static_cast<uint8_t>(lhs);
    let rhs_ = static_cast<uint8_t>(rhs);
    return (lhs_ & rhs_) == rhs_;
}


struct KeyboardEvent {
    enum class Type : uint8_t {
        Idle,
        PartialGrapheme, ///< The user is combining a grapheme.
        Grapheme, ///< The user has finished entering a grapheme.
        Key, ///< Key (+modifiers) was used to send a key.
    };

    Type type;
    KeyboardState state;

    Text::Grapheme grapheme;
    KeyboardKey key;

    KeyboardEvent() noexcept :
        type(Type::Idle), state(), grapheme(), key() {}

    /** Create a key-key keyboard event.
     */
    KeyboardEvent(KeyboardState state, KeyboardModifiers modifiers, char key) noexcept :
        type(Type::Key), state(state), grapheme(), key(modifiers, key) {}

    KeyboardEvent(Text::Grapheme grapheme, bool full=true) noexcept :
        type(full ? Type::Grapheme : Type::PartialGrapheme), state(), grapheme(std::move(grapheme)), key() {}

};

}
