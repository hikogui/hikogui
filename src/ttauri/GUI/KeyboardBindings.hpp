// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "KeyboardKey.hpp"
#include "../URL.hpp"
#include "../os_detect.hpp"
#include "../command.hpp"
#include <unordered_map>
#include <tuple>

namespace tt {

class KeyboardBindings {
    struct commands_t {
        /** Loading bindings from system-binding-file. */
        std::vector<command> system = {};

        /** Ignored system bindings loaded from user-binding-file. */
        std::vector<command> ignored = {};

        /** Added bindings loaded from user-binding-file. */
        std::vector<command> user = {};

        /** Combined system-/ignored-/added-commands. */
        std::vector<command> cache = {};

        [[nodiscard]] std::vector<command> const &get_commands() const noexcept {
            return cache;
        }

        void add_system_command(command cmd) noexcept {
            ttlet i = std::find(system.cbegin(), system.cend(), cmd);
            if (i == system.cend()) {
                system.push_back(cmd);
                update_cache();
            }
        }

        void add_ignored_command(command cmd) noexcept {
            ttlet i = std::find(ignored.cbegin(), ignored.cend(), cmd);
            if (i == ignored.cend()) {
                ignored.push_back(cmd);
                update_cache();
            }
        }

        void add_user_command(command cmd) noexcept {
            ttlet i = std::find(user.cbegin(), user.cend(), cmd);
            if (i == user.cend()) {
                user.push_back(cmd);
                update_cache();
            }
        }

        void update_cache() noexcept {
            cache.reserve(std::ssize(system) + std::ssize(user));

            for (ttlet cmd: system) {
                ttlet i = std::find(cache.cbegin(), cache.cend(), cmd);
                if (i == cache.cend()) {
                    cache.push_back(cmd);
                }
            }

            for (ttlet cmd: ignored) {
                ttlet i = std::find(cache.cbegin(), cache.cend(), cmd);
                if (i != cache.cend()) {
                    cache.erase(i);
                }
            }

            for (ttlet cmd: user) {
                ttlet i = std::find(cache.cbegin(), cache.cend(), cmd);
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

    void addSystemBinding(KeyboardKey key, command command) noexcept {
        bindings[key].add_system_command(command);
    }

    void addIgnoredBinding(KeyboardKey key, command command) noexcept {
        bindings[key].add_ignored_command(command);
    }

    void addUserBinding(KeyboardKey key, command command) noexcept {
        bindings[key].add_user_command(command);
    }

    /** translate a key press in the empty-context to a command.
    */
    [[nodiscard]] std::vector<command> const &translate(KeyboardKey key) const noexcept {
        static std::vector<command> empty_commands = {};

        ttlet i = bindings.find(key);
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
        if constexpr (OperatingSystem::current == OperatingSystem::Windows) {
            return loadBindings(URL{"resource:win32.keybinds.json"}, true);
        } else {
            tt_no_default();
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

/** Global keyboard bindings.
*/
inline KeyboardBindings keyboardBindings;

}
