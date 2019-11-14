// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Foundation/Options.hpp"
#include "TTauri/Foundation/logger.hpp"
#include <fmt/format.h>

namespace TTauri {

using namespace std::literals;

Options::Options(std::vector<OptionConfig> const &optionConfig, std::vector<std::string> const &arguments) noexcept
{
    for (let &config: optionConfig) {
        switch (config.type) {
        case OptionConfig::Type::Boolean:
            required_assert(config.default_value.is_boolean());
            break;
        case OptionConfig::Type::Integer:
            required_assert(config.default_value.is_integer());
            break;
        case OptionConfig::Type::String:
            required_assert(config.default_value.is_string());
            break;
        case OptionConfig::Type::URL:
            required_assert(config.default_value.is_url());
            break;
        case OptionConfig::Type::ListOfStrings:
            required_assert(config.default_value.is_vector());
            break;
        case OptionConfig::Type::LogLevel:
            required_assert(config.default_value.is_integer());
            break;
        default:
            no_default;
        }

        options[config.name] = config.default_value;
    }

    for (let &argument: arguments) {
        if (starts_with(argument, "--"s)) {
            let i = argument.find('=');
            if (i == argument.npos) {
                let option_name = argument.substr(2);

                let &config = std::find_if(optionConfig.begin(), optionConfig.end(), [&](auto x) {
                    return x.name == option_name;
                });

                if (config == optionConfig.end()) {
                    errorMessages.push_back(fmt::format("Unknown option '{}'", option_name));

                } else if (config->type != OptionConfig::Type::Boolean) {
                    errorMessages.push_back(fmt::format("Option '{}' requires an argument", option_name));
                
                } else {
                    options[option_name] = true;
                }

            } else {
                let option_name = argument.substr(2, i);
                let option_value_string = argument.substr(i+1);

                let &config = std::find_if(optionConfig.begin(), optionConfig.end(), [&](auto x) {
                    return x.name == option_name;
                });

                if (config == optionConfig.end()) {
                    errorMessages.push_back(fmt::format("Unknown option '{}'", option_name));

                } else {
                    switch (config->type) {
                    case OptionConfig::Type::Boolean:
                        if (option_value_string == "true") {
                            options[option_name] = true;
                        } else if (option_value_string == "false") {
                            options[option_name] = false;
                        } else {
                            errorMessages.push_back(
                                fmt::format("Expected a boolean value ('true' or 'false') for option '{}' got '{}'", option_name, option_value_string)
                            );
                        }
                        break;

                    case OptionConfig::Type::Integer:
                        try {
                            let option_value_int = std::stoll(option_value_string);
                            options[option_name] = option_value_int;
                        } catch (...) {
                            errorMessages.push_back(
                                fmt::format("Expected a integer value for option '{}' got '{}'", option_name, option_value_string)
                            );
                        }
                        break;

                    case OptionConfig::Type::String:
                        options[option_name] = option_value_string;
                        break;

                    case OptionConfig::Type::ListOfStrings:
                        options[option_name].push_back(datum{option_value_string});
                        break;

                    case OptionConfig::Type::URL:
                        options[option_name] = URL(option_value_string);
                        break;

                    case OptionConfig::Type::LogLevel:
                        if (option_value_string == "debug") {
                            options[option_name] = static_cast<int>(log_level::Debug);
                        } else if (option_value_string == "info") {
                            options[option_name] = static_cast<int>(log_level::Debug);
                        } else if (option_value_string == "audit") {
                            options[option_name] = static_cast<int>(log_level::Audit);
                        } else if (option_value_string == "warning") {
                            options[option_name] = static_cast<int>(log_level::Warning);
                        } else if (option_value_string == "error") {
                            options[option_name] = static_cast<int>(log_level::Error);
                        } else if (option_value_string == "critical") {
                            options[option_name] = static_cast<int>(log_level::Critical);
                        } else if (option_value_string == "fatal") {
                            options[option_name] = static_cast<int>(log_level::Fatal);
                        } else {
                            errorMessages.push_back(
                                fmt::format("Expected a log level value for option '{}' got '{}'", option_name, option_value_string)
                            );
                        }
                        break;                        

                    default:
                        no_default;
                    }
                }
            }

        } else {
            this->arguments.push_back(argument);
        }
    }
}

}