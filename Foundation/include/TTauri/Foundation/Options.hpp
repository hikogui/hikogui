// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/datum.hpp"
#include <string>
#include <vector>
#include <map>

namespace TTauri {

struct OptionConfig {
    enum class Type {
        URL,
        String,
        ListOfStrings,
        Integer,
        Boolean,
        LogLevel
    };

    std::string name;
    Type type;
    datum default_value;
    std::string help;
};

class Options {
    std::vector<std::string> errorMessages;
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