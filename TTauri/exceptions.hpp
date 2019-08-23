// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "required.hpp"
#include "string_tag.hpp"
#include "small_map.hpp"
#include "logger.hpp"
#include "any_repr.hpp"
#include "counters.hpp"
#include "logger.hpp"
#include <fmt/format.h>
#include <fmt/ostream.h>
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
public:
    char const *source_file;
    int source_line;

private:
    std::string _message;
    small_map<string_tag,std::any,16> _error_info = {};

protected:
    mutable std::string _what;

public:
    template<typename Fmt, typename... Args>
    error(Fmt const &fmt, Args const &... args) noexcept :
        _message(fmt::format(fmt, args...)) {}

    virtual string_tag tag() const noexcept = 0;

    /*! Return the name of the exception.
     */
    std::string name() const noexcept {
        return tag_to_string(tag());
    }

    std::string error_info_string() const noexcept {
        std::string r;

        int i = 0;
        for (let &info: _error_info) {
            if (i++ > 0) { r += " ,"; };
            r += fmt::format("({0}: {1})", tag_to_string(info.first), any_repr(info.second));   
        }
        return r;
    }

    void prepare_what() const noexcept {
        // XXX Strip off project directory from file.
        _what = fmt::format("{0},{1}:{2}: {3}. {4}",
            source_file,
            source_line,
            name(),
            _message,
            error_info_string()
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
    template<string_tag TAG, typename T>
    std::optional<T> get() const noexcept {
        if (let optional_info = _error_info.get(TAG)) {
            let &info = *optional_info;
            try {
                return std::any_cast<T>(info);
            } catch (std::bad_any_cast) {
                return {};
            }

        } else {
            return {};
        }
    }

    /*! Get a value from the exception.
    */
    template<string_tag TAG, typename T>
    T get(T default_value) const noexcept {
        if (let value = get<TAG,T>()) {
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
    if (!lhs._error_info.insert(rhs.tag, rhs.value)) {
        LOG_ERROR("Too many error_info values added to exception.");
    }
    return lhs;
}

template<typename T, typename O>
inline std::enable_if_t<std::is_base_of_v<error,T>, T> &operator<<(T &lhs, error_info_t<"source_file"_tag,O> const &rhs)
{
    lhs.source_file = rhs.value;
    return lhs;
}

template<typename T, typename O>
inline std::enable_if_t<std::is_base_of_v<error,T>, T> &operator<<(T &lhs, error_info_t<"source_line"_tag,O> const &rhs)
{
    lhs.source_line = rhs.value;
    return lhs;
}

#define TTAURI_THROW(x)\
    do {\
        auto e = (x)\
            << error_info<"source_file"_tag>(__FILE__)\
            << error_info<"source_line"_tag>(__LINE__)\
        ;\
        increment_counter<e.TAG>();\
        logger.log(log_level::Exception, __FILE__, int{__LINE__}, "{}", e.message());\
        throw e;\
    } while(false)

template<string_tag _TAG>
class sub_error : public error {
public:
    static constexpr string_tag TAG = _TAG;

    template<typename Fmt, typename... Args>
    sub_error(Fmt const &fmt, Args const &... args) noexcept :
        error(fmt, args...) {}

    string_tag tag() const noexcept override { return TAG; }
};

/*! Error to throw when parsing some kind of document.
 * This should be the primary exception to throw when there is an error in the document.
 *
 * It is important to check for all possible errors in a document and throw this error.
 * Since documents are often under user or advisary control we don't want to terminate
 * the application or worse compromise its security.
 *
 * For this reasons ParserErrors should not be ignored by the callees of a parser.
 */
using parse_error = sub_error<"parse_error"_tag>;

#define parse_assert(x) if (!(x)) { TTAURI_THROW(parse_error("{0}", #x )); }
#define parse_assert2(x, msg) if (!(x)) { TTAURI_THROW(parse_error(msg)); }

using url_error = sub_error<"url_error"_tag>;
using io_error = sub_error<"io_error"_tag>;
using key_error = sub_error<"key_error"_tag>;
using gui_error = sub_error<"gui_error"_tag>;
using bounds_error = sub_error<"bounds_error"_tag>;

/*! Error to throw when an operation can not be executed due to the type of its operants.
* This is for example used in universal_type.
*/
using invalid_operation_error = sub_error<"invalid_op"_tag>;

}
