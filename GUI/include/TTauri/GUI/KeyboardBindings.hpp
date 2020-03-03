// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/GUI/KeyboardKey.hpp"
#include "TTauri/Foundation/string_tag.hpp"
#include "TTauri/Foundation/URL.hpp"
#include <unordered_map>
#include <tuple>

namespace std {

template<>
struct hash<std::pair<TTauri::string_tag,TTauri::GUI::KeyboardKey>> {
    [[nodiscard]] size_t operator() (std::pair<TTauri::string_tag,TTauri::GUI::KeyboardKey> const &rhs) const noexcept {
        return TTauri::hash_mix(rhs.first, rhs.second);
    }
};

}

namespace TTauri::GUI {

class KeyboardBindings {
    /** All known key-bindings.
     * key = context,key
     * value = command,system-binding
     */
    std::unordered_map<std::pair<string_tag,KeyboardKey>, std::pair<string_ltag,bool>> bindings;

public:
    KeyboardBindings() noexcept :
        bindings() {}

    void addBinding(string_tag context, KeyboardKey key, string_ltag command, bool system_binding) noexcept {
        bindings[std::pair{context,key}] = std::pair{command,system_binding};
    }

    void addSystemBinding(string_tag context, KeyboardKey key, string_ltag command) noexcept {
        addBinding(context, key, command, true);
    }

    void addUserBinding(string_tag context, KeyboardKey key, string_ltag command) noexcept {
        addBinding(context, key, command, false);
    }

    /** translate a key press in the empty-context to a command.
    */
    [[nodiscard]] string_ltag translate(KeyboardKey key) const noexcept {
        let i = bindings.find(std::pair{string_tag{},key});
        return (i != bindings.cend()) ? i->second.first : string_ltag{};
    }

    /** translate a key press in a context to a command.
     * This will first search the current context, before trying the empty context.
     */
    [[nodiscard]] string_ltag translate(string_tag context, KeyboardKey key) const noexcept {
        let i = bindings.find(std::pair{context,key});
        return (i != bindings.cend()) ? i->second.first : translate(key);
    }

    /** Clear all bindings.
     * When loading a new user-binding file, one should
     * do a `clear()` followed by loading the system bindings, followed by the
     * user bindings.
     */
    void clear() noexcept {
        bindings.clear();
    }

    /** Load bindings from a JSON file.
     */
    void loadBindings(URL url);

    /** Load system bindings.
    */
    void loadSystemBindings() noexcept;

    void loadUserBindings(URL url) {
        clear();
        loadSystemBindings();
        loadBindings(url);
    }

    /** Save user bindings
     * This will save all bindings that are different from the system bindings.
     */
    void saveUserBindings(URL url);
};


}