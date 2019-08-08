// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "required.hpp"
#include <boost/exception/all.hpp>
#include <exception>
#include <string>

namespace TTauri {

struct URL;

using errinfo_parse_string = boost::error_info<struct tag_parse_string,std::string>;
using errinfo_url = boost::error_info<struct tag_url,URL>;

struct Error : virtual boost::exception, virtual std::exception {
    std::string _what;

     Error() noexcept : _what("unknown error") {}
     Error(const std::string& what) noexcept : _what(what) {}

    const char* what() const noexcept override {
        return _what.data();
    }
};

struct ParseError : Error {
     ParseError(const std::string& msg) noexcept: Error(msg) {}
    ParseError() noexcept : Error() {}
};

struct URLError : Error {
     URLError(const std::string& msg) noexcept : Error(msg) {}
    URLError() noexcept : Error() {}
};

struct FileError : Error {
     FileError(const std::string& msg) noexcept : Error(msg) {}
    FileError() noexcept : Error() {}
};

struct NotImplementedError : Error {
     NotImplementedError(const std::string& msg) noexcept : Error(msg) {}
    NotImplementedError() noexcept : Error() {}
};

struct OutOfBoundsError : Error {
     OutOfBoundsError(const std::string& msg) noexcept : Error(msg) {}
    OutOfBoundsError() noexcept : Error() {}
};

struct InvalidOperationError : Error {
     InvalidOperationError(const std::string& msg) noexcept : Error(msg) {}
    InvalidOperationError() noexcept : Error() {}
};


}