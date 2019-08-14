// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "required.hpp"
#include "string_tag.hpp"
#include <boost/format.hpp>
#include <fmt/format.h>
#include <exception>
#include <string>
#include <atomic>
#include <any>
#include <unordered_map>
#include <optional>

namespace TTauri {

template<string_tag TAG, typename T>
struct error_info_t {
    static constexpr string_tag tag = TAG;
    T value;

    error_info_t(T value) : value(std::move(value)) {}
};

template<string_tag TAG, typename T>
inline auto error_info(T value) {
    return error_info_t<TAG,T>(value);
}

class error : public std::exception {
    std::string _message;
    std::unordered_map<string_tag,std::any> _error_info = {};

protected:
    mutable std::string _what;

public:
    template<typename Fmt, typename... Args>
    error(Fmt const &fmt, Args const &... args) noexcept :
        _message(fmt::format(fmt, args...)) {}

    /*! Return the name of the exception.
     */
    virtual std::string name() const noexcept = 0;

    virtual void prepare_what() const noexcept {
        // XXX Strip off project directory from file.
        _what = fmt::format("{0},{1}:{2}: {3}.",
            get<"file"_tag>("<unknown>"),
            get<"line"_tag>(0),
            name(),
            _message
        );
    }

    const char* what() const noexcept override {
        prepare_what();
        return _what.data();
    }

    std::string message() const {
        return _message;
    }

    /*! Get a value from the exception.
    */
    template<typename T, string_tag TAG>
    std::optional<T> get() const noexcept {
        let i = _error_info.find(TAG);
        if (i == _error_info.end()) {
            return {};
        }

        let &any_value = i->second;
        try {
            return std::any_cast<T>(any_value);
        } catch (std::bad_any_cast) {
            return {};
        }
    }

    /*! Get a value from the exception.
    */
    template<string_tag TAG, typename T>
    T get(T default_value) const noexcept {
        if (let value = get<T,TAG>()) {
            return *value;
        } else {
            return default_value;
        }
    }

    template<typename T, string_tag TAG, typename O>
    friend std::enable_if_t<std::is_base_of_v<error,T>, T> &operator<<(T &lhs, error_info_t<TAG,O> const &rhs);
};


template<typename T, string_tag TAG, typename O>
inline std::enable_if_t<std::is_base_of_v<error,T>, T> &operator<<(T &lhs, error_info_t<TAG,O> const &rhs)
{
    lhs._error_info[rhs.tag] = rhs.value;
    return lhs;
}

#define TTAURI_THROW(x) throw x << error_info<"line"_tag>(int{__LINE__}) << error_info<"file"_tag>(__FILE__)

/*! Error to throw when parsing some kind of document.
 * This should be the primary exception to throw when there is an error in the document.
 *
 * It is important to check for all possible errors in a document and throw this error.
 * Since documents are often under user or advisary control we don't want to terminate
 * the application or worse compromise its security.
 *
 * For this reasons ParserErrors should not be ignored by the callees of a parser.
 */
class parse_error : public error {
public:
    template<typename Fmt, typename... Args>
    parse_error(Fmt const &fmt, Args const &... args) noexcept :
        error(fmt, args...) {}

    std::string name() const noexcept override { return "parse_error"; };
};

#define parse_assert(x) if (!(x)) { TTAURI_THROW(parse_error("{0}", #x )); }
#define parse_assert2(x, msg) if (!(x)) { TTAURI_THROW(parse_error(msg)); }

class url_error : public error {
public:
    template<typename Fmt, typename... Args>
    url_error(Fmt const &fmt, Args const &... args) noexcept :
        error(fmt, args...) {}

    std::string name() const noexcept override { return "url_error"; };
};

class io_error : public error {
public:
    template<typename Fmt, typename... Args>
    io_error(Fmt const &fmt, Args const &... args) noexcept :
        error(fmt, args...) {}

    std::string name() const noexcept override { return "io_error"; };
    void prepare_what() const noexcept override;
};

class key_error : public error {
public:
    template<typename Fmt, typename... Args>
    key_error(Fmt const &fmt, Args const &... args) noexcept :
        error(fmt, args...) {}

    std::string name() const noexcept override { return "key_error"; };
};

class index_error : public error {
public:
    template<typename Fmt, typename... Args>
    index_error(Fmt const &fmt, Args const &... args) noexcept :
        error(fmt, args...) {}

    std::string name() const noexcept override { return "index_error"; };
};

class gui_error : public error {
public:
    template<typename Fmt, typename... Args>
    gui_error(Fmt const &fmt, Args const &... args) noexcept :
        error(fmt, args...) {}

    std::string name() const noexcept override { return "gui_error"; };
};

/*! Error to throw when functionality was not implemented.
 */
class not_implemented_error : public error {
public:
    template<typename Fmt, typename... Args>
    not_implemented_error(Fmt const &fmt, Args const &... args) noexcept :
        error(fmt, args...) {}

    std::string name() const noexcept override { return "not_implemented_error"; };
};

class out_of_bounds_error : public error {
public:
    template<typename Fmt, typename... Args>
    out_of_bounds_error(Fmt const &fmt, Args const &... args) noexcept :
        error(fmt, args...) {}

    std::string name() const noexcept override { return "out_of_bounds_error"; };
};

/*! Error to throw when an operation can not be executed due to the type of its operants.
 * This is for example used in universal_type.
 */
class invalid_operation_error : public error {
public:
    template<typename Fmt, typename... Args>
    invalid_operation_error(Fmt const &fmt, Args const &... args) noexcept :
        error(fmt, args...) {}

    std::string name() const noexcept override { return "invalid_operation_error"; };
};

}
