// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "keyboard_bindings.hpp"
#include "../codec/JSON.hpp"
#include "../command.hpp"

namespace tt {

void keyboard_bindings::loadBindings(URL url, bool system_binding)
{
    ttlet data = parse_JSON(url);

    try {
        tt_parse_check(data.contains("bindings"), "Missing key 'bindings' at top level.");

        ttlet binding_list = data["bindings"];
        tt_parse_check(binding_list.is_vector(), "Expecting array value for key 'bindings' at top level.");

        for (auto i = binding_list.vector_begin(); i != binding_list.vector_end(); ++i) {
            ttlet binding = *i;
            tt_parse_check(binding.is_map(), "Expecting object for a binding, got {}", binding);

            tt_parse_check(
                binding.contains("key") && binding.contains("command"),
                "Expecting required 'key' and 'command' for a binding, got {}", binding
            );

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
                addIgnoredBinding(key, command);
            } else if (system_binding) {
                addSystemBinding(key, command);
            } else {
                addUserBinding(key, command);
            }
        }

    } catch (std::exception const &e) {
        throw io_error("{}: Could not load keyboard bindings.\n{}", url, e.what());
    }

}


}
