// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/GUI/KeyboardModifiers.hpp"
#include "TTauri/Foundation/hash.hpp"
#include "TTauri/Foundation/exceptions.hpp"
#include "TTauri/Foundation/strings.hpp"
#include <string_view>
#include <unordered_map>

namespace TTauri::GUI {

class KeyboardKey {
    /** Which modifiers where used on the key when sending a key.
    */
    KeyboardModifiers modifiers;

    /** ASCII code of the key that was pressed when sending a key.
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

    inline static std::unordered_map<std::string,char> key_names = {};

    constexpr KeyboardKey() noexcept :
        modifiers(KeyboardModifiers::None), key(0) {}

    constexpr KeyboardKey(KeyboardModifiers modifiers, char key) noexcept :
        modifiers(modifiers), key(key) {}

    constexpr KeyboardKey(uint16_t value) noexcept :
        modifiers(static_cast<KeyboardModifiers>(value >> 8)), key(static_cast<char>(value & 0xff))  {}

    KeyboardKey(std::string key_combination) :
        modifiers(KeyboardModifiers::None), key(Nul)
    {
        for (let &key_name: split(key_combination, '+')) {
            if (key_name == "shift") {
                modifiers |= KeyboardModifiers::Shift;
            } else if (key_name == "control") {
                modifiers |= KeyboardModifiers::Control;
            } else if (key_name == "alt") {
                modifiers |= KeyboardModifiers::Alt;
            } else if (key_name == "super") {
                modifiers |= KeyboardModifiers::Super;
            } else if (key == Nul) {
                let i = key_names.find(key_name);
                if (i != key_names.cend()) {
                    key = i->second;
                } else {
                    TTAURI_THROW(parse_error("Unknown key name '{}'", key_name));
                }
            } else {
                TTAURI_THROW(parse_error("Multiple non-modifier key found in '{}'", key_combination));
            }
        }
    }

    constexpr operator uint16_t () const noexcept {
        return (static_cast<uint16_t>(static_cast<uint8_t>(modifiers)) << 8) | static_cast<uint16_t>(key);
    }

    size_t hash() const noexcept {
        return hash_mix_two(std::hash<KeyboardModifiers>{}(modifiers), std::hash<char>{}(key));
    }

    [[nodiscard]] friend bool operator==(KeyboardKey const &lhs, KeyboardKey const &rhs) noexcept {
        return lhs.modifiers == rhs.modifiers && lhs.key == rhs.key;
    }
    [[nodiscard]] friend bool operator!=(KeyboardKey const &lhs, KeyboardKey const &rhs) noexcept {
        return !(lhs == rhs);
    }
};

}

namespace std {

template<>
struct hash<TTauri::GUI::KeyboardKey> {
    [[nodiscard]] size_t operator() (TTauri::GUI::KeyboardKey const &rhs) const noexcept {
        return rhs.hash();
    }
};

}