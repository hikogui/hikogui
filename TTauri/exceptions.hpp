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
#include "datum.hpp"
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <exception>
#include <string>
#include <atomic>
#include <unordered_map>
#include <optional>
#include <ostream>

namespace TTauri {

class error {
protected:
    std::string _message;
    small_map<string_tag,datum,4> error_info = {};

public:
    template<typename Fmt, typename... Args>
    error(Fmt const &fmt, Args &&... args) noexcept :
        _message(fmt::format(fmt, std::forward<Args>(args)...)) {}

    virtual string_tag tag() const noexcept = 0;

    /*! Return the name of the exception.
     */
    std::string name() const noexcept {
        return tag_to_string(tag());
    }

    std::string error_info_string() const noexcept {
        std::string r;

        int i = 0;
        for (let &info: error_info) {
            if (i++ > 0) { r += " ,"; };
            r += fmt::format("({}: {})", tag_to_string(info.key), info.value.repr());   
        }
        return r;
    }

    std::string string() const noexcept {
        return fmt::format("{}: {}. {}",
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
    error &set(InfoValueType &&info_value) noexcept {
        if (!error_info.insert(InfoTag, datum(std::forward<InfoValueType>(info_value)))) {
            LOG_ERROR("Too many error_info values added to exception.");
        }
        return *this;
    }

    /*! Get a value from the exception.
    */
    template<string_tag InfoTag, typename InfoValueType>
    std::optional<InfoValueType> get() const noexcept {
        if (let optional_info = error_info.get(InfoTag)) {
            let &info = *optional_info;

            try {
                return static_cast<InfoValueType>(info);
            } catch (...) {
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

inline std::ostream &operator<<(std::ostream &os, error const &x)
{
    return os << x.string();
}

template<string_tag _TAG>
class sub_error : public error {
public:
    static constexpr string_tag TAG = _TAG;

    template<typename Fmt, typename... Args>
    sub_error(Fmt const &fmt, Args &&... args) noexcept :
        error(fmt, std::forward<Args>(args)...) {}

    /*!
     * A non-virtual method like this will return the actual class instance
     * which means throw knows exactly which class is being thrown.
     */
    template<string_tag InfoTag, typename InfoValueType>
    sub_error &set(InfoValueType &&info_value) noexcept {
        if (!error_info.insert(InfoTag, datum(std::forward<InfoValueType>(info_value)))) {
            LOG_ERROR("Too many error_info values added to exception.");
        }
        return *this;
    }

    sub_error &log(char const *source_file, int source_line) {
        logger.log<log_level::Exception>(source_file, source_line, "{}", *this);
        increment_counter<TAG>();
        return *this;
    }

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

using url_error = sub_error<"url_error"_tag>;
using io_error = sub_error<"io_error"_tag>;
using key_error = sub_error<"key_error"_tag>;
using gui_error = sub_error<"gui_error"_tag>;
using bounds_error = sub_error<"bounds_error"_tag>;

/*! Error to throw when an operation can not be executed due to the type of its operants.
* This is for example used in universal_type.
*/
using invalid_operation_error = sub_error<"invalid_op"_tag>;

#define TTAURI_THROW(x) throw std::move((x).log(__FILE__, __LINE__));
    


#define parse_assert(x) if (!(x)) { TTAURI_THROW(parse_error("{0}", #x )); }
#define parse_assert2(x, msg) if (!(x)) { TTAURI_THROW(parse_error(msg)); }

}
