// Copyright 2020 Pokitec
// All rights reserved.

#include "TTauri/GUI/KeyboardBindings.hpp"
#include "TTauri/Foundation/JSON.hpp"

namespace TTauri::GUI {

void KeyboardBindings::loadBindings(URL url, bool system_binding)
{
    let data = parseJSON(url);

    try {
        parse_assert2(data.contains("bindings"), "Missing key 'bindings' at top level.");

        let bindings = data["bindings"];
        parse_assert2(bindings.is_vector(), "Expecting array value for key 'bindings' at top level.");

        for (auto i = bindings.vector_begin(); i != bindings.vector_end(); ++i) {
            let binding = *i;
            parse_assert2(binding.is_map(), "Expecting object for a binding, got {}", binding);

            parse_assert2(
                binding.contains("key") && binding.contains("command"),
                "Expecting required 'key' and 'command' for a binding, got {}", binding
            );

            let key_name = binding["key"];
            let command_name = binding["command"];
            let context_name = binding.contains("context") ? binding["context"] : datum{""};

            let key = KeyboardKey(static_cast<std::string>(key_name));

            auto command_tag = string_ltag{};
            try {
                command_tag = tt5_encode<string_ltag>(static_cast<std::string>(command_name));
            } catch (parse_error &e) {
                TTAURI_THROW(parse_error("Could not parse command '{}'", command_name).caused_by(e));
            }

            auto context_tag = string_tag{};
            try {
                context_tag = tt5_encode<string_tag>(static_cast<std::string>(context_name));
            } catch (parse_error &e) {
                TTAURI_THROW(parse_error("Could not parse context '{}'", context_name).caused_by(e));
            }
            addBinding(context_tag, key, command_tag, system_binding);
        }

    } catch (error &e) {
        e.set<"url"_tag>(url);
        throw;
    }

}


}