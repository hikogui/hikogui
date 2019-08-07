// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include <boost/exception/all.hpp>
#include <exception>
#include <string>

namespace TTauri {

struct URL;

using errinfo_parse_string = boost::error_info<struct tag_parse_string,std::string>;
using errinfo_url = boost::error_info<struct tag_url,URL>;

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
    ParseError() : Error("") {}
};

struct URLError : Error {
    URLError(const std::string& msg) : Error(msg) {}
    URLError() : Error("") {}
};

struct FileError : Error {
    FileError(const std::string& msg) : Error(msg) {}
    FileError() : Error("") {}
};

struct NotImplementedError : Error {
    NotImplementedError(const std::string& msg) : Error(msg) {}
    NotImplementedError() : Error("") {}
};

struct OutOfBoundsError : Error {
    OutOfBoundsError(const std::string& msg) : Error(msg) {}
    OutOfBoundsError() : Error("") {}
};

struct InvalidOperationError : Error {
    InvalidOperationError(const std::string& msg) : Error(msg) {}
    InvalidOperationError() : Error("") {}
};


}