// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "keyboard_key.hpp"
#include "gui_event.hpp"
#include "../utility/module.hpp"
#include <unordered_map>
#include <tuple>
#include <filesystem>

namespace hi::inline v1 {

class keyboard_bindings {
    struct commands_t {
        /** Loading bindings from system-binding-file. */
        std::vector<gui_event_type> system = {};

        /** Ignored system bindings loaded from user-binding-file. */
        std::vector<gui_event_type> ignored = {};

        /** Added bindings loaded from user-binding-file. */
        std::vector<gui_event_type> user = {};

        /** Combined system-/ignored-/added-commands. */
        std::vector<gui_event> cache = {};

        [[nodiscard]] std::vector<gui_event> const& get_events() const noexcept
        {
            return cache;
        }

        void add_system_command(gui_event_type cmd) noexcept
        {
            hilet i = std::find(system.cbegin(), system.cend(), cmd);
            if (i == system.cend()) {
                system.push_back(cmd);
                update_cache();
            }
        }

        void add_ignored_command(gui_event_type cmd) noexcept
        {
            hilet i = std::find(ignored.cbegin(), ignored.cend(), cmd);
            if (i == ignored.cend()) {
                ignored.push_back(cmd);
                update_cache();
            }
        }

        void add_user_command(gui_event_type cmd) noexcept
        {
            hilet i = std::find(user.cbegin(), user.cend(), cmd);
            if (i == user.cend()) {
                user.push_back(cmd);
                update_cache();
            }
        }

        void update_cache() noexcept
        {
            cache.reserve(ssize(system) + ssize(user));

            for (hilet cmd : system) {
                hilet i = std::find(cache.cbegin(), cache.cend(), cmd);
                if (i == cache.cend()) {
                    cache.emplace_back(cmd);
                }
            }

            for (hilet cmd : ignored) {
                hilet i = std::find(cache.cbegin(), cache.cend(), cmd);
                if (i != cache.cend()) {
                    cache.erase(i);
                }
            }

            for (hilet cmd : user) {
                hilet i = std::find(cache.cbegin(), cache.cend(), cmd);
                if (i == cache.cend()) {
                    cache.emplace_back(cmd);
                }
            }
        }
    };

    /** Bindings made by the user which may be saved for the user.
     */
    std::unordered_map<keyboard_key, commands_t> bindings;

public:
    keyboard_bindings() noexcept : bindings() {}

    void add_system_binding(keyboard_key key, gui_event_type command) noexcept
    {
        bindings[key].add_system_command(command);
    }

    void add_ignored_binding(keyboard_key key, gui_event_type command) noexcept
    {
        bindings[key].add_ignored_command(command);
    }

    void add_user_binding(keyboard_key key, gui_event_type command) noexcept
    {
        bindings[key].add_user_command(command);
    }

    /** translate a key press in the empty-context to a command.
     *
     * @param event The event to look up in the bindings.
     * @param[in,out] events The event list to append the bindings to when found.
     */
    [[nodiscard]] void translate(gui_event event, std::vector<gui_event>& events) const noexcept
    {
        if (event == gui_event_type::keyboard_down) {
            hilet i = bindings.find(keyboard_key{event.keyboard_modifiers, event.key()});
            if (i != bindings.cend()) {
                hilet& new_events = i->second.get_events();
                events.insert(events.cend(), new_events.cbegin(), new_events.cend());
            }
        }
    }

    /** Clear all bindings.
     * When loading a new user-binding file, one should
     * do a `clear()` followed by loading the system bindings, followed by the
     * user bindings.
     */
    void clear() noexcept
    {
        bindings.clear();
    }

    /** Load bindings from a JSON file.
     */
    void load_bindings(std::filesystem::path const &path, bool system_binding = false);

    /** Save user bindings
     * This will save all bindings that are different from the system bindings.
     */
    // void save_user_bindings(std::filesystem::path const &path);
};

} // namespace hi::inline v1
