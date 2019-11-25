// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/datum.hpp"
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <string_view>
#include <iostream>

namespace TTauri {

using namespace std::literals;

/** A parser to parse command line arguments.
 */
class CommandLineParser {
    /** Specification of possible command line option.
    */
    struct option_t {
        /** Name of the option.
        */
        std::string name;

        /** Type of the option.
        */
        datum_type_t type;

        /** Help message for the option.
        */
        std::string help;

        /** A function to decode a string into an integer.
         * This is mostly useful to for enums.
         */
        std::function<int(std::string_view)> enum_conversion;

        option_t(std::string name, datum_type_t type, std::string help, std::function<int(std::string_view)> enum_conversion) :
            name(std::move(name)), type(type), help(std::move(help)), enum_conversion(std::move(enum_conversion)) {}
    };

    // The synopsis of the application to be printed on --help and error.
    std::string synopsis;

    // A list of options.
    std::vector<option_t> options;

    /// A list of error messages occured during parsing.
    std::vector<std::string> error_messages;

public:
    CommandLineParser(std::string synopsis) : synopsis(std::move(synopsis)) {}

    void add(std::string name, datum_type_t type, std::string help, std::function<int(std::string_view)> enum_conversion = {}) noexcept {
        options.emplace_back(std::move(name), type, std::move(help), std::move(enum_conversion));
    }

    bool has_error() const noexcept {
        return error_messages.size() > 0;
    }

    void print_help() {
        for (let &error_message: error_messages) {
            std::cerr << error_message << "\n";
        }
        if (has_error()) {
            std::cerr << "\n";
        }

        std::cerr << synopsis << "\n";

        for (let &option: options) {
            let example = fmt::format("--{}=<{}>", option.name, option.type);
            std::cerr << fmt::format("  {:20s}    {}\n", example, option.help);
        }
        std::cerr.flush();
    }

    datum parse(std::vector<std::string> const &arguments) noexcept {
        auto configuration = datum{datum::map{}};

        bool firstArgument = true;
        for (let &argument: arguments) {
            if (firstArgument) {
                firstArgument = true;
                configuration["executable-path"] = arguments[0];

            } else if (starts_with(argument, "--"s)) {
                let i = argument.find('=');
                if (i == argument.npos) {
                    let option_name = argument.substr(2);

                    let &option = std::find_if(options.begin(), options.end(), [&](auto x) {
                        return x.name == option_name;
                        });

                    if (option == options.end()) {
                        error_messages.push_back(fmt::format("Unknown option '{}'", option_name));

                    } else if (option->type != datum_type_t::Boolean) {
                        error_messages.push_back(fmt::format("Option '{}' requires an argument", option_name));

                    } else {
                        configuration[option_name] = true;
                    }

                } else {
                    let option_name = argument.substr(2, i);
                    let option_value_string = argument.substr(i+1);

                    let &option = std::find_if(options.begin(), options.end(), [&](auto x) {
                        return x.name == option_name;
                        });

                    if (option == options.end()) {
                        error_messages.push_back(fmt::format("Unknown option '{}'", option_name));

                    } else {
                        switch (option->type) {
                        case datum_type_t::Boolean:
                            if (option_value_string == "true") {
                                configuration[option_name] = true;
                            } else if (option_value_string == "false") {
                                configuration[option_name] = false;
                            } else {
                                error_messages.push_back(
                                    fmt::format("Expected a boolean value ('true' or 'false') for option '{}' got '{}'", option_name, option_value_string)
                                );
                            }
                            break;

                        case datum_type_t::Integer:
                            if (option->enum_conversion) {
                                let option_value_int = option->enum_conversion(option_value_string);
                                if (option_value_int >= 0) {
                                    configuration[option_name] = option_value_int;
                                } else {
                                    error_messages.push_back(
                                        fmt::format("Unknown value '{}' for option '{}'", option_value_string, option_name)
                                    );
                                }

                            } else {
                                try {
                                    let option_value_int = std::stoll(option_value_string);
                                    configuration[option_name] = option_value_int;
                                } catch (...) {
                                    error_messages.push_back(
                                        fmt::format("Expected a integer value for option '{}' got '{}'", option_name, option_value_string)
                                    );
                                }
                            }
                            break;

                        case datum_type_t::String:
                            configuration[option_name] = option_value_string;
                            break;

                        case datum_type_t::Vector:
                            configuration[option_name].push_back(datum{option_value_string});
                            break;

                        case datum_type_t::URL:
                            configuration[option_name] = URL(option_value_string);
                            break; 

                        default:
                            no_default;
                        }
                    }
                }

            } else {
                configuration["arguments"].push_back(argument);
            }
        }
        return configuration;
    }
};

}
