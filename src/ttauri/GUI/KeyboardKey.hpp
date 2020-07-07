// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/GUI/KeyboardModifiers.hpp"
#include "TTauri/GUI/KeyboardVirtualKey.hpp"
#include "ttauri/hash.hpp"
#include "ttauri/exceptions.hpp"
#include "ttauri/strings.hpp"
#include <string_view>
#include <unordered_map>

namespace tt {





/** A key in combination with modifiers.
 * This key is based on the actual symbol on the keyboard.
 */
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
    KeyboardVirtualKey virtualKey;

public:
    constexpr KeyboardKey() noexcept :
        modifiers(KeyboardModifiers::None), virtualKey(KeyboardVirtualKey::Nul) {}

    constexpr KeyboardKey(KeyboardModifiers modifiers, KeyboardVirtualKey key) noexcept :
        modifiers(modifiers), virtualKey(key) {}

    KeyboardKey(std::string_view key_combination) :
        modifiers(KeyboardModifiers::None), virtualKey(KeyboardVirtualKey::Nul)
    {
        ttlet modifiers_and_vkey = split(key_combination, '+');
        tt_assert(modifiers_and_vkey.cbegin() != modifiers_and_vkey.cend());

        ttlet end_modifiers = modifiers_and_vkey.cend() - 1;
        for (auto i = modifiers_and_vkey.cbegin(); i != end_modifiers; ++i) {
            modifiers |= to_KeyboardModifiers(*i);
        }

        virtualKey = to_KeyboardVirtualKey(modifiers_and_vkey.back());
    }

    size_t hash() const noexcept {
        return hash_mix(modifiers, virtualKey);
    }

    [[nodiscard]] friend bool operator==(KeyboardKey const &lhs, KeyboardKey const &rhs) noexcept {
        return lhs.modifiers == rhs.modifiers && lhs.virtualKey == rhs.virtualKey;
    }
    [[nodiscard]] friend bool operator!=(KeyboardKey const &lhs, KeyboardKey const &rhs) noexcept {
        return !(lhs == rhs);
    }

    [[nodiscard]] friend std::string to_string(KeyboardKey const &rhs) noexcept {
        return fmt::format("{}{}", rhs.modifiers, rhs.virtualKey);
    }

    friend std::ostream &operator<<(std::ostream &lhs, KeyboardKey const &rhs) {
        return lhs << to_string(rhs);
    }
};

}

namespace std {

template<>
struct hash<tt::KeyboardKey> {
    [[nodiscard]] size_t operator() (tt::KeyboardKey const &rhs) const noexcept {
        return rhs.hash();
    }
};

}