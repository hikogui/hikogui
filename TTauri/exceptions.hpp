// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include <boost/exception/all.hpp>
#include <exception>
#include <string>

namespace TTauri {

struct Error : virtual boost::exception, virtual std::exception {
    std::string _what;

    Error() : _what("unknown error") {}
    Error(const std::string& what) : _what(what) {}

    const char* what() const noexcept override {
        return _what.data();
    }
};

struct ParseError : Error {
    ParseError(const std::string& msg) : Error(msg) {}
};

}