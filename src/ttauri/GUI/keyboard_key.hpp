// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "keyboard_modifiers.hpp"
#include "keyboard_virtual_key.hpp"
#include "../hash.hpp"
#include "../exception.hpp"
#include "../strings.hpp"
#include <string_view>
#include <unordered_map>

namespace tt {
inline namespace v1 {

/** A key in combination with modifiers.
 * This key is based on the actual symbol on the keyboard.
 */
class keyboard_key {
    /** Which modifiers where used on the key when sending a key.
    */
    keyboard_modifiers modifiers;

    /** ASCII code of the key that was pressed when sending a key.
    *
    * All printable ASCII characters are mapped to the equivalent key on the
    * keyboard, after processing of the shift-key.
    *
    * The following non-printable ASCII characters are used 
    */
    keyboard_virtual_key virtualKey;

public:
    constexpr keyboard_key() noexcept :
        modifiers(keyboard_modifiers::None), virtualKey(keyboard_virtual_key::Nul) {}

    constexpr keyboard_key(keyboard_modifiers modifiers, keyboard_virtual_key key) noexcept :
        modifiers(modifiers), virtualKey(key) {}

    keyboard_key(std::string_view key_combination) :
        modifiers(keyboard_modifiers::None), virtualKey(keyboard_virtual_key::Nul)
    {
        ttlet modifiers_and_vkey = split(key_combination, '+');
        tt_assert(modifiers_and_vkey.cbegin() != modifiers_and_vkey.cend());

        ttlet end_modifiers = modifiers_and_vkey.cend() - 1;
        for (auto i = modifiers_and_vkey.cbegin(); i != end_modifiers; ++i) {
            modifiers |= to_keyboard_modifiers(*i);
        }

        virtualKey = to_keyboard_virtual_key(modifiers_and_vkey.back());
    }

    size_t hash() const noexcept {
        return hash_mix(modifiers, virtualKey);
    }

    [[nodiscard]] friend bool operator==(keyboard_key const &lhs, keyboard_key const &rhs) noexcept {
        return lhs.modifiers == rhs.modifiers && lhs.virtualKey == rhs.virtualKey;
    }
    [[nodiscard]] friend bool operator!=(keyboard_key const &lhs, keyboard_key const &rhs) noexcept {
        return !(lhs == rhs);
    }

    [[nodiscard]] friend std::string to_string(keyboard_key const &rhs) noexcept {
        return std::format("{}{}", rhs.modifiers, rhs.virtualKey);
    }

    friend std::ostream &operator<<(std::ostream &lhs, keyboard_key const &rhs) {
        return lhs << to_string(rhs);
    }
};

}
}

template<>
struct std::hash<tt::keyboard_key> {
    [[nodiscard]] size_t operator() (tt::keyboard_key const &rhs) const noexcept {
        return rhs.hash();
    }
};

