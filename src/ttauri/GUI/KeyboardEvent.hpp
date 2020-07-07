// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/GUI/KeyboardKey.hpp"
#include "ttauri/text/Grapheme.hpp"
#include "ttauri/required.hpp"
#include "ttauri/assert.hpp"
#include <utility>

namespace tt {

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
    ttlet lhs_ = static_cast<uint8_t>(lhs);
    ttlet rhs_ = static_cast<uint8_t>(rhs);
    return (lhs_ & rhs_) == rhs_;
}


struct KeyboardEvent {
    enum class Type : uint8_t {
        Idle,
        Entered, ///< Keyboard focus was given.
        Exited, ///< Keyboard focus was taken away.
        PartialGrapheme, ///< The user is combining a grapheme.
        Grapheme, ///< The user has finished entering a grapheme.
        Key, ///< Key (+modifiers) was used to send a key.
    };

    Type type;
    KeyboardState state;

    Grapheme grapheme;
    KeyboardKey key;

    KeyboardEvent(Type type=Type::Idle) noexcept :
        type(type), state(), grapheme(), key() {}

    /** Create a key-key keyboard event.
     */
    KeyboardEvent(KeyboardState state, KeyboardModifiers modifiers, KeyboardVirtualKey key) noexcept :
        type(Type::Key), state(state), grapheme(), key(modifiers, key) {}

    KeyboardEvent(Grapheme grapheme, bool full=true) noexcept :
        type(full ? Type::Grapheme : Type::PartialGrapheme), state(), grapheme(std::move(grapheme)), key() {}

    [[nodiscard]] static KeyboardEvent entered() noexcept {
        return KeyboardEvent(Type::Entered);
    }

    [[nodiscard]] static KeyboardEvent exited() noexcept {
        return KeyboardEvent(Type::Exited);
    }

    [[nodiscard]] std::vector<string_ltag> const &getCommands() const noexcept;

    [[nodiscard]] friend std::string to_string(KeyboardEvent const &rhs) noexcept {
        auto r = std::string{"<KeyboardEvent "};

        switch (rhs.type) {
        case Type::Idle: r += "Idle"; break;
        case Type::Entered: r += "Entered"; break;
        case Type::Exited: r += "Exited"; break;
        case Type::PartialGrapheme: r += "PartialGrapheme="; break;
        case Type::Grapheme: r += "Grapheme="; break;
        case Type::Key: r += "Key="; break;
        default: tt_no_default;
        }

        if (rhs.type == Type::PartialGrapheme || rhs.type == Type::Grapheme) {
            r += to_string(rhs.grapheme);
        } else if (rhs.type == Type::Key) {
            r += to_string(rhs.key);
        }

        r += ">";
        return r;
    }

    friend std::ostream &operator<<(std::ostream &lhs, KeyboardEvent const &rhs) {
        return lhs << to_string(rhs);
    }
};

}
