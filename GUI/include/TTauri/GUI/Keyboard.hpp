// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

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

/** Key modificatio keys pressed at the same time as another key.
 *
 * The Fn key is not always avilable on larger keyboards and is often under full
 * control of the keyboard, therefor it is not in the list of keyboard modifiers here.
 */
enum class KeyboardModifiers : uint8_t {
    None = 0x00,
    Shift = 0x01,   ///< The shift key is being held.
    Control = 0x02, ///< The control key is being held.
    Alt = 0x04,     ///< The alt-key, option-key or meta-key is being held.
    Super = 0x08,   ///< The windows-key, command-key or super-key is being held.
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

/** Convert a constant string to a KeyboardModifiers instance.
 * - '_': Shift
 * - '^': Control
 * - 'A': Alt
 * - 'S': Super
 */
[[nodiscard]] constexpr KeyboardModifiers to_KeyboardModifiers(char const *modifiers) noexcept
{
    auto r = KeyboardModifiers::Idle;

    for (ssize_t i = 0; modifiers[i] != 0; ++i) {
        switch (modifiers[i]) {
        case '_': r |= KeyboardModifiers::Shift; break;
        case '^': r |= KeyboardModifiers::Control; break;
        case 'A': r |= KeyboardModifiers::Alt; break;
        case 'S': r |= KeyboardModifiers::Super; break;
        default: no_default;
        }
    }

    return r;
}


class CommandKey {
    /** Which modifiers where used on the key when sending a command.
    */
    KeyboardModifiers modifiers;

    /** ASCII code of the key that was pressed when sending a command.
    *
    * All printable ASCII characters are mapped to the equivalent key on the
    * keyboard, after processing of the shift-key.
    *
    * The following non-printable ASCII characters are used 
    */
    char key;

public:
    constexpr static char Nul = 0x00; // ASCII Null
    constexpr static char Print = 0x01;
    constexpr static char Home = 0x02; // ASCII start-of-text
    constexpr static char End = 0x03; // ASCII end-of-text
    constexpr static char LeftArrow = 0x04;
    constexpr static char RightArrow = 0x05; 
    constexpr static char UpArrow = 0x06; 
    constexpr static char DownArrow = 0x07; 
    constexpr static char Backspace = 0x08; // ASCII backspace
    constexpr static char Tab = 0x09; // ASCII tab
    constexpr static char Enter = 0x0a; // ASCII new line
    constexpr static char F1 = 0x0b; 
    constexpr static char F2 = 0x0c; 
    constexpr static char F3 = 0x0d; 
    constexpr static char F4 = 0x0e; 
    constexpr static char F5 = 0x0f; 
    constexpr static char F6 = 0x10; 
    constexpr static char F7 = 0x11; 
    constexpr static char F8 = 0x12; 
    constexpr static char F9 = 0x13; 
    constexpr static char F10 = 0x14;
    constexpr static char F11 = 0x15; 
    constexpr static char F12 = 0x16;
    constexpr static char Clear = 0x17; // NumPad 5.
    constexpr static char PauseBreak = 0x18; // ASCII cancel
    constexpr static char VolumeMute = 0x19;
    constexpr static char Insert = 0x1a; // ASCII substitute
    constexpr static char Escape = 0x1b; // ASCII escape 
    constexpr static char PageUp = 0x1c; 
    constexpr static char PageDown = 0x1d;
    constexpr static char VolumeUp = 0x1e;
    constexpr static char VolumeDown = 0x1f;
    constexpr static char Delete = 0x7f;

    constexpr CommandKey() noexcept :
        modifiers(KeyboardModifiers::Idle), key(0) {}

    constexpr CommandKey(KeyboardModifiers modifiers, char key) noexcept :
        modifiers(modifiers), key(key) {}

    constexpr CommandKey(uint16_t value) noexcept :
        modifiers(static_cast<KeyboardModifiers>(value >> 8)), key(static_cast<char>(value & 0xff))  {}

    constexpr CommandKey(char const *modifiers, char key) noexcept :
        modifiers(to_KeyboardModifiers(modifiers)), key(key) {}

    constexpr operator uint16_t () const noexcept {
        return (static_cast<uint16_t>(static_cast<uint8_t>(modifiers)) << 8) | static_cast<uint16_t>(key);
    }
};

struct KeyboardEvent {
    enum class Type : uint8_t {
        Idle,
        PartialGrapheme, ///< The user is combining a grapheme.
        Grapheme, ///< The user has finished entering a grapheme.
        Command, ///< Key (+modifiers) was used to send a command.
    };

    Type type;
    KeyboardState state;

    Text::Grapheme grapheme;
    CommandKey command;

    KeyboardEvent() noexcept :
        type(Type::Idle), state(), grapheme(), command() {}

    /** Create a command-key keyboard event.
     */
    KeyboardEvent(KeyboardState state, KeyboardModifiers modifiers, char key) noexcept :
        type(Type::Command), state(state), grapheme(), command(modifiers, key) {}

    KeyboardEvent(Text::Grapheme grapheme, bool full=true) noexcept :
        type(full ? Type::Grapheme : Type::PartialGrapheme), state(), grapheme(std::move(grapheme)), command() {}

    /** Convert the keyboard-event to a text-edit-command.
     * This function will be specific for the operating it is running on.
     */
    [[nodiscard]] TextEditCommand getTextEditCommand() noexcept;
};

}
