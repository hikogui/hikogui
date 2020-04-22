// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/GUI/KeyboardKey.hpp"
#include "TTauri/Foundation/string_tag.hpp"
#include "TTauri/Foundation/URL.hpp"
#include "TTauri/Foundation/os_detect.hpp"
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
    struct commands_t {
        /** Loading bindings from system-binding-file. */
        std::vector<string_ltag> system = {};

        /** Ignored system bindings loaded from user-binding-file. */
        std::vector<string_ltag> ignored = {};

        /** Added bindings loaded from user-binding-file. */
        std::vector<string_ltag> user = {};

        /** Combined system-/ignored-/added-commands. */
        std::vector<string_ltag> cache = {};

        [[nodiscard]] std::vector<string_ltag> const &get_commands() const noexcept {
            return cache;
        }

        void add_system_command(string_ltag cmd) noexcept {
            let i = std::find(system.cbegin(), system.cend(), cmd);
            if (i == system.cend()) {
                system.push_back(cmd);
                update_cache();
            }
        }

        void add_ignored_command(string_ltag cmd) noexcept {
            let i = std::find(ignored.cbegin(), ignored.cend(), cmd);
            if (i == ignored.cend()) {
                ignored.push_back(cmd);
                update_cache();
            }
        }

        void add_user_command(string_ltag cmd) noexcept {
            let i = std::find(user.cbegin(), user.cend(), cmd);
            if (i == user.cend()) {
                user.push_back(cmd);
                update_cache();
            }
        }

        void update_cache() noexcept {
            cache.reserve(ssize(system) + ssize(user));

            for (let cmd: system) {
                let i = std::find(cache.cbegin(), cache.cend(), cmd);
                if (i == cache.cend()) {
                    cache.push_back(cmd);
                }
            }

            for (let cmd: ignored) {
                let i = std::find(cache.cbegin(), cache.cend(), cmd);
                if (i != cache.cend()) {
                    cache.erase(i);
                }
            }

            for (let cmd: user) {
                let i = std::find(cache.cbegin(), cache.cend(), cmd);
                if (i == cache.cend()) {
                    cache.push_back(cmd);
                }
            }
        }
    };

    /** Bindings made by the user which may be saved for the user.
     */
    std::unordered_map<KeyboardKey,commands_t> bindings;

public:
    KeyboardBindings() noexcept :
        bindings() {}

    void addSystemBinding(KeyboardKey key, string_ltag command) noexcept {
        bindings[key].add_system_command(command);
    }

    void addIgnoredBinding(KeyboardKey key, string_ltag command) noexcept {
        bindings[key].add_ignored_command(command);
    }

    void addUserBinding(KeyboardKey key, string_ltag command) noexcept {
        bindings[key].add_user_command(command);
    }

    /** translate a key press in the empty-context to a command.
    */
    [[nodiscard]] std::vector<string_ltag> const &translate(KeyboardKey key) const noexcept {
        static std::vector<string_ltag> empty_commands = {};

        let i = bindings.find(key);
        if (i != bindings.cend()) {
            return i->second.get_commands();
        } else {
            return empty_commands;
        }
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
    void loadBindings(URL url, bool system_binding);

    /** Load system bindings.
    */
    void loadSystemBindings() {
        if constexpr (operatingSystem == OperatingSystem::Windows) {
            return loadBindings(URL{"resource:Themes/win32.keybinds.json"}, true);
        } else {
            no_default;
        }
    }

    void loadUserBindings(URL url) {
        clear();
        loadSystemBindings();
        loadBindings(url, false);
    }

    /** Save user bindings
     * This will save all bindings that are different from the system bindings.
     */
    void saveUserBindings(URL url);
};


}