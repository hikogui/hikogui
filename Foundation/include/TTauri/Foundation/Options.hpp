// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/datum.hpp"
#include <string>
#include <vector>
#include <map>

namespace TTauri {

/*! Specification of an config option.
 */
struct OptionConfig {
    enum class Type {
        URL,
        String,
        ListOfStrings,
        Integer,

        /*! A boolean option may be set to true without an argument value.
         */
        Boolean,
        LogLevel
    };

    /*! Name of the option.
     */
    std::string name;

    /*! Type of the option.
     */
    Type type;

    /*! Default value of the option.
     */
    datum default_value;

    /*! Help message for the option.
     */
    std::string help;
};

/*! Options parsed from command line arguments and configuration file.
 */
class Options {
    std::vector<std::string> errorMessages;
    std::string executable;
    std::vector<std::string> arguments;

    std::map<std::string,datum> options;
    datum null_datum = {};

public:
    Options(std::vector<OptionConfig> const &optionConfig, std::vector<std::string> const &arguments) noexcept;

    [[nodiscard]] datum const &operator[](std::string const &name) const noexcept {
        auto i = options.find(name);
        if (i == options.end()) {
            return null_datum;
        } else {
            return i->second;
        }
    }
};

}
