// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "required.hpp"
#include <boost/exception/all.hpp>
#include <exception>
#include <string>
#include <atomic>

namespace TTauri {
struct URL;

struct error : public std::exception {
    std::string _what;
    std::any _causedBy = {};
    std::unordered_map<std::string,std::any> error_info = {};

    error(const std::string& what) noexcept : _what(what) {}
    error() noexcept : _what() {}

    const char* what() const noexcept override {
        return _what.data();
    }

    template<typename E>
    error &causedBy(E const &e) noexcept {
        _causedBy = e;
        return *this;
    }

    /*! Add a value to the exception.
     */
    template<typename T>
    error& add(std::string const &name, T const &value) noexcept {
        error_info[name] = value;
        return *this;
    }

    /*! Check if the exception has a value.
     */
    bool contains(std::string const &name) const noexcept {
        let i = error_info.find(name);
        return i != error_info.end();
    }

    /*! Get a value from the exception.
     */
    template<typename T>
    std::optional<T> try_get(std::string const &name) const noexcept {
        let i = error_info.find(name);
        if (i == error_info.end()) {
            return {};
        }
        
        let &any_value = i->second;
        try {
            return any_cast<T>(any_value);
        } catch (std:bad_any_cast const &e) {
            return {};
        }
    }

    /*! Get a value from the exception.
     */
    template<typename T>
    T get(std::string const &name) const noexcept {
        if (let value = try_get<T>(name)) {
            return value;
        } else {
            std::terminate();
        }
    }
};

#include TTAURI_THROW(x) throw (x).add("line", __LINE__).add("file", __FILE__)

/*! Error to throw when parsing some kind of document.
 * This should be the primary exception to throw when there is an error in the document.
 *
 * It is important to check for all possible errors in a document and throw this error.
 * Since documents are often under user or advisary control we don't want to terminate
 * the application or worse compromise its security.
 *
 * For this reasons ParserErrors should not be ignored by the callees of a parser.
 */
struct parse_error : public error {
    parse_error(const std::string& msg) noexcept: error(msg) {}
    parse_error() noexcept : error() {}
};

#define parse_assert(x) if (!(x)) { TTAURI_THROW(parse_error( #x )); }
#define parse_assert2(x, msg) if (!(x)) { TTAURI_THROW(parse_error(msg)); }

struct url_error : public error {
    url_error(const std::string& msg) noexcept : error(msg) {}
    url_error() noexcept : error() {}
};

struct file_error : public error {
    file_error(const std::string& msg) noexcept : error(msg) {}
    file_error() noexcept : error() {}
};

/*! Error to throw when functionality was not implemented.
 */
struct not_implemented_error : public error {
    not_implemented_error(const std::string& msg) noexcept : error(msg) {}
    not_implemented_error() noexcept : error() {}
};

struct out_of_bounds_error : public error {
    out_of_bounds_error(const std::string& msg) noexcept : error(msg) {}
    out_of_bounds_error() noexcept : error() {}
};

/*! Error to throw when an operation can not be executed due to the type of its operants.
 * This is for example used in universal_type.
 */
struct invalid_operation_error : public error {
    invalid_operation_error(const std::string& msg) noexcept : error(msg) {}
    invalid_operation_error() noexcept : error() {}
};

}
