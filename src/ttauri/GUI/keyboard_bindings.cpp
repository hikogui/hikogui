// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "keyboard_bindings.hpp"
#include "../codec/JSON.hpp"
#include "../command.hpp"
#include "../log.hpp"

namespace tt {

void keyboard_bindings::load_bindings(URL url, bool system_binding)
{
    ttlet data = parse_JSON(url);

    try {
        tt_parse_check(data.contains("bindings"), "Missing key 'bindings' at top level.");

        ttlet binding_list = data["bindings"];
        tt_parse_check(holds_alternative<datum::vector_type>(binding_list), "Expecting array value for key 'bindings' at top level.");

        for (ttlet &binding: binding_list) {
            tt_parse_check(holds_alternative<datum::map_type>(binding), "Expecting object for a binding, got {}", binding);

            tt_parse_check(
                binding.contains("key") && binding.contains("command"),
                "Expecting required 'key' and 'command' for a binding, got {}",
                binding);

            ttlet key_name = static_cast<std::string>(binding["key"]);
            ttlet key = keyboard_key(key_name);

            auto command_name = static_cast<std::string>(binding["command"]);

            // Commands starting with '-' are ignored system-bindings.
            bool ignored_binding = false;
            if (command_name.size() >= 1 && command_name[0] == '-') {
                ignored_binding = true;
                command_name = command_name.substr(1);
            }

            auto command = to_command(command_name);
            if (command == command::unknown) {
                throw parse_error("Could not parse command '{}'", command_name);
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
        throw io_error("{}: Could not load keyboard bindings.\n{}", url, e.what());
    }
}

} // namespace tt
