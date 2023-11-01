// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"

#include <unordered_map>
#include <tuple>
#include <filesystem>
#include <coroutine>

export module hikogui_GUI : keyboard_bindings;
import : gui_event;
import : keyboard_key;
import hikogui_codec;
import hikogui_coroutine;
import hikogui_utility;

export namespace hi { inline namespace v1 {

class keyboard_bindings {
public:
    keyboard_bindings() noexcept : bindings() {}

    static keyboard_bindings& global() noexcept;

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
     * @return The event list translated from the keyboard event.
     */
    [[nodiscard]] generator<gui_event> translate(gui_event event) const noexcept
    {
        if (event == gui_event_type::keyboard_down) {
            hilet i = bindings.find(keyboard_key{event.keyboard_modifiers, event.key()});
            if (i != bindings.cend()) {
                for (auto& e : i->second.get_events()) {
                    co_yield e;
                }
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
    void load_bindings(std::filesystem::path const& path, bool system_binding = false)
    {
        hilet data = parse_JSON(path);

        try {
            hi_check(data.contains("bindings"), "Missing key 'bindings' at top level.");

            hilet binding_list = data["bindings"];
            hi_check(
                holds_alternative<datum::vector_type>(binding_list), "Expecting array value for key 'bindings' at top level.");

            for (hilet& binding : binding_list) {
                hi_check(holds_alternative<datum::map_type>(binding), "Expecting object for a binding, got {}", binding);

                hi_check(
                    binding.contains("key") && binding.contains("command"),
                    "Expecting required 'key' and 'command' for a binding, got {}",
                    binding);

                hilet key_name = static_cast<std::string>(binding["key"]);
                hilet key = keyboard_key(key_name);

                auto command_name = static_cast<std::string>(binding["command"]);

                // Commands starting with '-' are ignored system-bindings.
                bool ignored_binding = false;
                if (command_name.size() >= 1 && command_name[0] == '-') {
                    ignored_binding = true;
                    command_name = command_name.substr(1);
                }

                auto command = to_gui_event_type(command_name);
                if (command == gui_event_type::none) {
                    throw parse_error(std::format("Could not parse command '{}'", command_name));
                }

                if (ignored_binding) {
                    add_ignored_binding(key, command);
                } else if (system_binding) {
                    add_system_binding(key, command);
                } else {
                    add_user_binding(key, command);
                }
            }

        } catch (std::exception const& e) {
            throw io_error(std::format("{}: Could not load keyboard bindings.\n{}", path.string(), e.what()));
        }
    }

    /** Save user bindings
     * This will save all bindings that are different from the system bindings.
     */
    // void save_user_bindings(std::filesystem::path const &path);

private:
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
};

namespace detail {
std::unique_ptr<keyboard_bindings> keyboard_bindings_global;
}

keyboard_bindings& keyboard_bindings::global() noexcept
{
    if (not detail::keyboard_bindings_global) {
        detail::keyboard_bindings_global = std::make_unique<keyboard_bindings>();
    }
    return *detail::keyboard_bindings_global;
}

void load_user_keyboard_bindings(std::filesystem::path const& path)
{
    return keyboard_bindings::global().load_bindings(path, false);
}

void load_system_keyboard_bindings(std::filesystem::path const& path)
{
    return keyboard_bindings::global().load_bindings(path, true);
}

generator<gui_event> translate_keyboard_event(gui_event event) noexcept
{
    for (auto& e : keyboard_bindings::global().translate(event)) {
        co_yield e;
    }
}

}} // namespace hi::v1
