// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "keyboard_bindings.hpp"
#include "../codec/JSON.hpp"
#include "gui_event_type.hpp"
#include "../log.hpp"

namespace hi::inline v1 {

void keyboard_bindings::load_bindings(std::filesystem::path const &path, bool system_binding)
{
    hilet data = parse_JSON(path);

    try {
        hi_check(data.contains("bindings"), "Missing key 'bindings' at top level.");

        hilet binding_list = data["bindings"];
        hi_check(
            holds_alternative<datum::vector_type>(binding_list), "Expecting array value for key 'bindings' at top level.");

        for (hilet &binding : binding_list) {
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

    } catch (std::exception const &e) {
        throw io_error(std::format("{}: Could not load keyboard bindings.\n{}", path.string(), e.what()));
    }
}

} // namespace hi::inline v1
