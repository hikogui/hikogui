// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/logger.hpp"
#include "TTauri/Foundation/counters.hpp"
#include "TTauri/Foundation/datum.hpp"
#include "TTauri/Foundation/required.hpp"
#include "TTauri/Foundation/string_tag.hpp"
#include "TTauri/Foundation/tagged_map.hpp"
#include "TTauri/Foundation/parse_location.hpp"
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <exception>
#include <string>
#include <atomic>
#include <unordered_map>
#include <optional>
#include <ostream>
#include <utility>

namespace TTauri {

struct url_tag {};
struct line_tag {};
struct column_tag {};
struct vk_result_tag {};
struct errno_tag {};
struct error_message_tag {};
struct key_tag {};

class error {
protected:
    std::string _message;

private:
    virtual error &set(std::type_index info_tag, datum const &info_value) noexcept = 0;

    virtual error &set(std::type_index info_tag, datum &&info_value) noexcept = 0;

    virtual datum &get(std::type_index info_tag) noexcept = 0;

    virtual datum const &get(std::type_index info_tag) const noexcept = 0;

    virtual bool has(std::type_index info_tag) const noexcept = 0;


public:
    template<typename Fmt, typename... Args>
    error(Fmt const &fmt, Args &&... args) noexcept {
        if constexpr (sizeof...(args) == 0) {
            _message = fmt;
        } else {
            _message = fmt::format(fmt, std::forward<Args>(args)...);
        }
    }

    virtual std::type_index tag() const noexcept = 0;
    virtual std::string error_info_string() const noexcept = 0;

    /*! Return the name of the exception.
    */
    std::string name() const noexcept {
        return tag_name(tag());
    }

    std::string string() const noexcept {
        return fmt::format("{}: {}. {}",
            name(),
            _message,
            error_info_string()
        );
    }

    [[nodiscard]] std::string message() const noexcept {
        return _message;
    }

    error &caused_by(error const &other) noexcept {
        _message = fmt::format("{}\nCaused by: {}", _message, other.string());
        return *this;
    }

    /*!
     * A non-virtual method like this will return the actual class instance
     * which means throw knows exactly which class is being thrown.
     */
    template<typename InfoTag, typename InfoValueType>
    error &set(InfoValueType &&info_value) noexcept {
        return set(std::type_index(typeid(InfoTag)), datum{std::forward<InfoValueType>(info_value)});
    }

    error &set_location(parse_location const &location) noexcept {
        if (location.has_file()) {
            set<url_tag>(location.file());
        }
        set<line_tag>(location.line());
        set<column_tag>(location.column());
        return *this;
    }

    /*! Merge locations.
     * Used when the current exception is a expression inside a statement.
     */
    error &merge_location(parse_location statement_location) noexcept {
        let line = static_cast<int>(get<line_tag>());
        let column = static_cast<int>(get<column_tag>());

        auto expression_location = parse_location{line, column};
        if (has<url_tag>()) {
            let url = static_cast<URL>(get<url_tag>());
            expression_location.set_file(std::move(url));
        }

        statement_location += expression_location;

        if (statement_location.has_file()) {
            set<url_tag>(statement_location.file());
        }
        set<line_tag>(statement_location.line());
        set<column_tag>(statement_location.column());
        return *this;
    }


    template<typename InfoTag>
    datum &get() noexcept {
        return get(std::type_index(typeid(InfoTag)));
    }

    template<typename InfoTag>
    datum const &get() const noexcept {
        return get(std::type_index(typeid(InfoTag)));
    }

    template<typename InfoTag>
    bool has() const noexcept {
        return has(std::type_index(typeid(InfoTag)));
    }

    friend std::string to_string(error const &rhs) {
        return rhs.string();
    }

    friend std::ostream &operator<<(std::ostream &os, error const &rhs) {
        return os << to_string(rhs);
    }

};


template<typename Tag, typename... InfoTags>
class sub_error final : public error {
    tagged_map<datum,InfoTags...> error_info = {};

    error &set(std::type_index info_tag, datum const &info_value) noexcept override {
        if (error_info.has(info_tag) == 1) {
            error_info.get(info_tag) = info_value;
        } else {
            LOG_WARNING("Unknown error_info '{}' on error '{}'", info_tag.name(), std::type_index(typeid(Tag)).name());
        }
        return *this;
    }

    error &set(std::type_index info_tag, datum &&info_value) noexcept override {
        if (error_info.has(info_tag) == 1) {
            error_info.get(info_tag) = std::move(info_value);
        } else {
            LOG_WARNING("Unknown error_info '{}' on error '{}'", info_tag.name(), std::type_index(typeid(Tag)).name());
        }
        return *this;
    }

    datum &get(std::type_index info_tag) noexcept override {
        ttauri_assert(error_info.has(info_tag) == 1);
        return error_info.get(info_tag);
    }

    datum const &get(std::type_index info_tag) const noexcept override {
        ttauri_assert(error_info.has(info_tag) == 1);
        return error_info.get(info_tag);
    }

    bool has(std::type_index info_tag) const noexcept override {
        if (error_info.has(info_tag) == 0) {
            return false;
        } else {
            return !error_info.get(info_tag).is_undefined();
        }
    }


public:
    using TAG = Tag;

    std::type_index tag() const noexcept override { return std::type_index(typeid(Tag)); }

    template<typename Fmt, typename... Args>
    sub_error(Fmt const &fmt, Args &&... args) noexcept :
        error(fmt, std::forward<Args>(args)...) {}

    sub_error &caused_by(error const &other) noexcept {
        _message = fmt::format("{}\nCaused by: {}", _message, other.string());
        return *this;
    }

    /*!
     * A non-virtual method like this will return the actual class instance
     * which means throw knows exactly which class is being thrown.
     */
    template<typename InfoTag, typename InfoValueType>
    sub_error &set(InfoValueType &&info_value) noexcept {
        static_assert(has_tag<InfoTag,InfoTags...>(), "Unknown tag of error info value.");
        error_info.template get<InfoTag>() = std::forward<InfoValueType>(info_value);
        return *this;
    }

    sub_error &set_location(parse_location const &location) noexcept {
        if (location.has_file()) {
            set<url_tag>(location.file());
        }
        set<line_tag>(location.line());
        set<column_tag>(location.column());
        return *this;
    }

    template<typename InfoTag>
    datum &get() noexcept {
        static_assert(has_tag<InfoTag,InfoTags...>(), "Unknown tag of error info value.");
        return error_info.template get<InfoTag>();
    }

    template<typename InfoTag>
    datum const &get() const noexcept {
        static_assert(has_tag<InfoTag,InfoTags...>(), "Unknown tag of error info value.");
        return error_info.template get<InfoTag>();
    }

    template<typename InfoTag>
    bool has() const noexcept {
        static_assert(has_tag<InfoTag,InfoTags...>(), "Unknown tag of error info value.");
        return !((error_info.template get<InfoTag>()).is_undefined());
    }

    sub_error &log(char const *source_file, int source_line) {
        logger.log<log_level::Exception>(cpu_counter_clock::now(), "{}", *this, source_code_ptr(source_file, source_line));
        increment_counter<Tag>();
        return *this;
    }

    std::string error_info_string() const noexcept override {
        std::string r;

        for (size_t i = 0; i < error_info.size(); i++) {
            if (i > 0) {
                r += ", ";
            };

            r += fmt::format("{}={}",
                tag_name(error_info.get_tag(i)),
                error_info[i].repr()
            );
        }
        return r;
    }
};


/*! Error to throw when parsing some kind of document.
 * This should be the primary exception to throw when there is an error in the document.
 *
 * It is important to check for all possible errors in a document and throw this error.
 * Since documents are often under user control we don't want to terminate
 * the application or worse compromise its security.
 *
 * For this reasons ParserErrors should not be ignored by the callees of a parser.
 */
struct parse_error_tag {};
using parse_error = sub_error<parse_error_tag, url_tag, line_tag, column_tag>;

/** Error to throw when an operation can not be executed due to the type of its operants.
** This is for example used in universal_type.
**/
struct invalid_operation_tag {};
using invalid_operation_error = sub_error<invalid_operation_tag, url_tag, line_tag, column_tag>;

struct url_error_tag {};
using url_error = sub_error<url_error_tag, url_tag>;
struct io_error_tag {};
using io_error = sub_error<io_error_tag, url_tag, errno_tag, error_message_tag>;
struct key_error_tag {};
using key_error = sub_error<key_error_tag, key_tag>;
struct gui_error_tag {};
using gui_error = sub_error<gui_error_tag, vk_result_tag>;
struct bounds_error_tag {};
using bounds_error = sub_error<bounds_error_tag>;
struct math_error_tag {};
using math_error = sub_error<math_error_tag>;


#define TTAURI_THROW(x) throw std::move((x).log(__FILE__, __LINE__));

#define parse_assert(x) if (!(x)) { TTAURI_THROW(parse_error("{}", #x )); }
#define parse_assert2(x, ...) if (!(x)) { TTAURI_THROW(parse_error(__VA_ARGS__)); }

#define hresult_assert_or_throw(x) ([](HRESULT result) {\
        if (ttauri_unlikely(FAILED(result))) {\
            TTAURI_THROW(io_error("Call to '{}' failed with {:08x}", #x, result));\
        }\
        return result;\
    }(x))


}
