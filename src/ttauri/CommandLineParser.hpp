// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "datum.hpp"
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <string_view>
#include <iostream>

namespace tt {

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

    /** Add a configuration option.
     *
     * @param name Name of the option. Excluding the leading dashes
     * @param type Type of the option's argument
     * @param help Description text of the option.
     * @param enum_conversion A function that converts a string to an integer.
     */
    void add(std::string name, datum_type_t type, std::string help, std::function<int(std::string_view)> enum_conversion = {}) noexcept {
        options.emplace_back(std::move(name), type, std::move(help), std::move(enum_conversion));
    }

    /** check if an error has occured during parsing.
     */
    bool has_error() const noexcept {
        return error_messages.size() > 0;
    }

    /** Print help text for the command line arguments.
     * This will also print any error messages that happened during the parsing.
     */
    void print_help() {
        for (ttlet &error_message: error_messages) {
            std::cerr << error_message << "\n";
        }
        if (has_error()) {
            std::cerr << "\n";
        }

        std::cerr << synopsis << "\n";

        for (ttlet &option: options) {
            ttlet example = fmt::format("--{}=<{}>", option.name, option.type);
            std::cerr << fmt::format("  {:20s}    {}\n", example, option.help);
        }
        std::cerr.flush();
    }

    /** Parse the arguments.
     * The result will be a map of option/value pairs.
     * Special options are:
     * - 'executable-path' The path to the exectuable.
     * - 'arguments' A list of strings of the non-option arguments.
     * 
     * @param arguments a list of command line arguments, including the exectable name as the first argument.
     * @return The result as a map-datum, with option names as the keys.
     */
    datum parse(std::vector<std::string> const &arguments) noexcept {
        auto r = datum{datum::map{}};

        int argumentCount = 0;
        for (ttlet &argument: arguments) {
            if (argumentCount++ == 0) {
                r["executable-path"] = arguments[0];

            } else if (argument.starts_with("--"s)) {
                ttlet i = argument.find('=');
                if (i == argument.npos) {
                    ttlet option_name = argument.substr(2);

                    ttlet &option = std::find_if(options.begin(), options.end(), [&](auto x) {
                        return x.name == option_name;
                        });

                    if (option == options.end()) {
                        error_messages.push_back(fmt::format("Unknown option '{}'", option_name));

                    } else if (option->type != datum_type_t::Boolean) {
                        error_messages.push_back(fmt::format("Option '{}' requires an argument", option_name));

                    } else {
                        r[option_name] = true;
                    }

                } else {
                    ttlet option_name = argument.substr(2, i-2);
                    ttlet option_value_string = argument.substr(i+1);

                    ttlet &option = std::find_if(options.begin(), options.end(), [&](auto x) {
                        return x.name == option_name;
                        });

                    if (option == options.end()) {
                        error_messages.push_back(fmt::format("Unknown option '{}'", option_name));

                    } else {
                        switch (option->type) {
                        case datum_type_t::Boolean:
                            if (option_value_string == "true") {
                                r[option_name] = true;
                            } else if (option_value_string == "false") {
                                r[option_name] = false;
                            } else {
                                error_messages.push_back(
                                    fmt::format("Expected a boolean value ('true' or 'false') for option '{}' got '{}'", option_name, option_value_string)
                                );
                            }
                            break;

                        case datum_type_t::Integer:
                            if (option->enum_conversion) {
                                ttlet option_value_int = option->enum_conversion(option_value_string);
                                if (option_value_int >= 0) {
                                    r[option_name] = option_value_int;
                                } else {
                                    error_messages.push_back(
                                        fmt::format("Unknown value '{}' for option '{}'", option_value_string, option_name)
                                    );
                                }

                            } else {
                                try {
                                    ttlet option_value_int = std::stoll(option_value_string);
                                    r[option_name] = option_value_int;
                                } catch (...) {
                                    error_messages.push_back(
                                        fmt::format("Expected a integer value for option '{}' got '{}'", option_name, option_value_string)
                                    );
                                }
                            }
                            break;

                        case datum_type_t::String:
                            r[option_name] = option_value_string;
                            break;

                        case datum_type_t::Vector:
                            r[option_name].push_back(datum{option_value_string});
                            break;

                        case datum_type_t::URL:
                            r[option_name] = URL::urlFromCurrentWorkingDirectory().urlByAppendingPath(option_value_string);
                            break; 

                        default:
                            tt_no_default;
                        }
                    }
                }

            } else {
                r["arguments"].push_back(argument);
            }
        }
        return r;
    }
};

}
