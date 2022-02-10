// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "keyboard_key.hpp"
#include "../unicode/grapheme.hpp"
#include "../geometry/transform.hpp"
#include "../required.hpp"
#include "../assert.hpp"
#include "../command.hpp"
#include <utility>

namespace tt::inline v1 {

enum class KeyboardState : uint8_t {
    Idle = 0x00,
    CapsLock = 0x01,
    ScrollLock = 0x02,
    NumLock = 0x04,
};

[[nodiscard]] constexpr KeyboardState operator|(KeyboardState lhs, KeyboardState rhs) noexcept
{
    return static_cast<KeyboardState>(static_cast<uint8_t>(lhs) | static_cast<uint8_t>(rhs));
}

constexpr KeyboardState &operator|=(KeyboardState &lhs, KeyboardState rhs) noexcept
{
    lhs = lhs | rhs;
    return lhs;
}

[[nodiscard]] constexpr bool operator>=(KeyboardState lhs, KeyboardState rhs) noexcept
{
    ttlet lhs_ = static_cast<uint8_t>(lhs);
    ttlet rhs_ = static_cast<uint8_t>(rhs);
    return (lhs_ & rhs_) == rhs_;
}

struct keyboard_event {
    enum class Type : uint8_t {
        Idle,
        partial_grapheme, ///< The user is combining a grapheme.
        grapheme, ///< The user has finished entering a grapheme.
        Key, ///< Key (+modifiers) was used to send a key.
    };

    Type type;
    KeyboardState state;

    tt::grapheme grapheme;
    keyboard_key key;

    keyboard_event(Type type = Type::Idle) noexcept : type(type), state(), grapheme(), key() {}

    /** Create a key-key keyboard event.
     */
    keyboard_event(KeyboardState state, keyboard_modifiers modifiers, keyboard_virtual_key key) noexcept :
        type(Type::Key), state(state), grapheme(), key(modifiers, key)
    {
    }

    keyboard_event(tt::grapheme grapheme, bool full = true) noexcept :
        type(full ? Type::grapheme : Type::partial_grapheme), state(), grapheme(std::move(grapheme)), key()
    {
    }

    [[nodiscard]] std::vector<command> const &getCommands() const noexcept;

    [[nodiscard]] friend std::string to_string(keyboard_event const &rhs) noexcept
    {
        auto r = std::string{"<keyboard_event "};

        switch (rhs.type) {
        case Type::Idle: r += "Idle"; break;
        case Type::partial_grapheme: r += "partial_grapheme="; break;
        case Type::grapheme: r += "grapheme="; break;
        case Type::Key: r += "Key="; break;
        default: tt_no_default();
        }

        if (rhs.type == Type::partial_grapheme || rhs.type == Type::grapheme) {
            r += to_string(rhs.grapheme);
        } else if (rhs.type == Type::Key) {
            r += to_string(rhs.key);
        }

        r += ">";
        return r;
    }

    friend std::ostream &operator<<(std::ostream &lhs, keyboard_event const &rhs)
    {
        return lhs << to_string(rhs);
    }
};

} // namespace tt::inline v1
