// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include <boost/exception/all.hpp>
#include <exception>
#include <string>

namespace TTauri::Config {

using errinfo_location = boost::error_info<struct tag_location,Location>;

struct ConfigError : virtual boost::exception, virtual std::exception {
    std::string _what;

    ConfigError() : _what("unknown ConfigError") {}
    ConfigError(const std::string &what) : _what(what) {}

    const char* what() const noexcept override {
        return _what.data();
    }
};

struct ParseError : ConfigError {
    ParseError(const std::string &msg) : ConfigError(msg) {}
};

struct InvalidOperationError : ConfigError {
    InvalidOperationError(const std::string &msg) : ConfigError(msg) {}
};

struct IOError : ConfigError {
    IOError(const std::string &msg) : ConfigError(msg) {}
};

struct InternalParserError : ConfigError {
    InternalParserError(const std::string &msg) : ConfigError(msg) {}
};

}
