// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include <boost/exception/all.hpp>
#include <exception>
#include <string>

namespace TTauri::Draw {

struct DrawError : virtual boost::exception, virtual std::exception {
    std::string _what;

    DrawError() : _what("unknown ConfigError") {}
    DrawError(const std::string& what) : _what(what) {}

    const char* what() const noexcept override {
        return _what.data();
    }
};

struct TrueTypeError : DrawError {
    TrueTypeError(const std::string& msg) : DrawError(msg) {}
};

}
