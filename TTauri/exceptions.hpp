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

class error {
public:
    char const *source_file;
    int source_line;

protected:
    std::string _message;
    small_map<string_tag,std::any,4> _error_info = {};

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

    std::string string() const noexcept {
        // XXX Strip off project directory from file.
        return fmt::format("{0},{1}:{2}: {3}. {4}",
            source_file,
            source_line,
            name(),
            _message,
            error_info_string()
        );
    }

    std::string message() const {
        return _message;
    }

    /*!
     * A non-virtual method like this will return the actual class instance
     * which means throw knows exactly which class is being thrown.
     */
    template<string_tag InfoTag, typename InfoValueType>
    error &set(InfoValueType const &infoValue) noexcept {
        if (!_error_info.insert(InfoTag, infoValue)) {
            LOG_ERROR("Too many error_info values added to exception.");
        }
        return *this;
    }

    /*! Get a value from the exception.
    */
    template<string_tag InfoTag, typename InfoValueType>
    std::optional<InfoValueType> get() const noexcept {
        if (let optional_info = _error_info.get(InfoTag)) {
            let &info = *optional_info;
            try {
                return std::any_cast<InfoValueType>(info);
            } catch (std::bad_any_cast) {
                return {};
            }

        } else {
            return {};
        }
    }

    /*! Get a value from the exception.
    */
    template<string_tag InfoTag, typename InfoValueType>
    InfoValueType get(InfoValueType default_value) const noexcept {
        if (let value = get<InfoTag,InfoValueType>()) {
            return *value;
        } else {
            return default_value;
        }
    }
};

template<string_tag _TAG>
class sub_error : public error {
public:
    static constexpr string_tag TAG = _TAG;

    template<typename Fmt, typename... Args>
    sub_error(Fmt const &fmt, Args const &... args) noexcept :
        error(fmt, args...) {}

    /*!
     * A non-virtual method like this will return the actual class instance
     * which means throw knows exactly which class is being thrown.
     */
    template<string_tag InfoTag, typename InfoValueType>
    sub_error &set(InfoValueType const &info_value) noexcept {
        if (!_error_info.insert(InfoTag, info_value)) {
            LOG_ERROR("Too many error_info values added to exception.");
        }
        return *this;
    }

    string_tag tag() const noexcept override { return TAG; }

    size_t test() {
        return sizeof(_error_info);
    }
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

using url_error = sub_error<"url_error"_tag>;
using io_error = sub_error<"io_error"_tag>;
using key_error = sub_error<"key_error"_tag>;
using gui_error = sub_error<"gui_error"_tag>;
using bounds_error = sub_error<"bounds_error"_tag>;

/*! Error to throw when an operation can not be executed due to the type of its operants.
* This is for example used in universal_type.
*/
using invalid_operation_error = sub_error<"invalid_op"_tag>;

#define TTAURI_THROW(x)\
    do {\
        auto e = (x);\
        e.source_file = __FILE__;\
        e.source_line = __LINE__;\
        increment_counter<e.TAG>();\
        logger.log<log_level::Exception>(__FILE__, __LINE__, "{}", e.test());\
        throw e;\
    } while(false)


#define parse_assert(x) if (!(x)) { TTAURI_THROW(parse_error("{0}", #x )); }
#define parse_assert2(x, msg) if (!(x)) { TTAURI_THROW(parse_error(msg)); }

}
