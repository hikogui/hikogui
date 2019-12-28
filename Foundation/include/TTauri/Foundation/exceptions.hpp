// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/logger.hpp"
#include "TTauri/Foundation/counters.hpp"
#include "TTauri/Foundation/datum.hpp"
#include "TTauri/Foundation/required.hpp"
#include "TTauri/Foundation/string_tag.hpp"
#include "TTauri/Foundation/tagged_map.hpp"
#include "TTauri/Foundation/Location.hpp"
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

class error {
private:
    std::string _message;

    virtual error &set(string_tag info_tag, datum const &info_value) noexcept = 0;

    virtual error &set(string_tag info_tag, datum &&info_value) noexcept = 0;

    virtual datum &get(string_tag info_tag) noexcept = 0;

    virtual datum const &get(string_tag info_tag) const noexcept = 0;

    virtual bool has(string_tag info_tag) const noexcept = 0;


public:
    template<typename Fmt, typename... Args>
    error(Fmt const &fmt, Args &&... args) noexcept {
        if constexpr (sizeof...(args) == 0) {
            _message = fmt;
        } else {
            _message = fmt::format(fmt, std::forward<Args>(args)...);
        }
    }

    virtual string_tag tag() const noexcept = 0;
    virtual std::string error_info_string() const noexcept = 0;

    /*! Return the name of the exception.
    */
    std::string name() const noexcept { return tag_to_string(tag()); }

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
        return set(InfoTag, datum{std::forward<InfoValueType>(info_value)});
    }

    template<string_tag InfoTag>
    datum &get() noexcept {
        return get(InfoTag);
    }

    template<string_tag InfoTag>
    datum const &get() const noexcept {
        return get(InfoTag);
    }

    template<string_tag InfoTag>
    bool has() const noexcept {
        return has(InfoTag);
    }

    friend std::string to_string(error const &rhs) {
        return rhs.string();
    }

    friend std::ostream &operator<<(std::ostream &os, error const &rhs) {
        return os << to_string(rhs);
    }

};


template<string_tag Tag, string_tag... InfoTags>
class sub_error final : public error {
    tagged_map<datum,InfoTags...> error_info = {};

    error &set(string_tag info_tag, datum const &info_value) noexcept override {
        if (error_info.has(info_tag) == 1) {
            error_info.get(info_tag) = info_value;
        } else {
            LOG_WARNING("Unknown error_info '{}' on error '{}'", tag_to_string(info_tag), tag_to_string(Tag));
        }
        return *this;
    }

    error &set(string_tag info_tag, datum &&info_value) noexcept override {
        if (error_info.has(info_tag) == 1) {
            error_info.get(info_tag) = std::move(info_value);
        } else {
            LOG_WARNING("Unknown error_info '{}' on error '{}'", tag_to_string(info_tag), tag_to_string(Tag));
        }
        return *this;
    }

    datum &get(string_tag info_tag) noexcept override {
        ttauri_assert(error_info.has(info_tag) == 1);
        return error_info.get(info_tag);
    }

    datum const &get(string_tag info_tag) const noexcept override {
        ttauri_assert(error_info.has(info_tag) == 1);
        return error_info.get(info_tag);
    }

    bool has(string_tag info_tag) const noexcept override {
        if (error_info.has(info_tag) == 0) {
            return false;
        } else {
            return !error_info.get(info_tag).is_undefined();
        }
    }


public:
    static constexpr string_tag TAG = Tag;

    string_tag tag() const noexcept override { return Tag; }

    template<typename Fmt, typename... Args>
    sub_error(Fmt const &fmt, Args &&... args) noexcept :
        error(fmt, std::forward<Args>(args)...) {}

    /*!
     * A non-virtual method like this will return the actual class instance
     * which means throw knows exactly which class is being thrown.
     */
    template<string_tag InfoTag, typename InfoValueType>
    sub_error &set(InfoValueType &&info_value) noexcept {
        static_assert(count_tag_if<InfoTags...>(InfoTag) == 1, "Unknown tag of error info value.");
        error_info.template get<InfoTag>() = std::forward<InfoValueType>(info_value);
        return *this;
    }

    sub_error &set_location(Location const &location) noexcept {
        if (location.has_file()) {
            set<"url"_tag>(location.file());
        }
        set<"line"_tag>(location.line());
        set<"column"_tag>(location.column());
        return *this;
    }

    template<string_tag InfoTag>
    datum &get() noexcept {
        static_assert(count_tag_if<InfoTags...>(InfoTag) == 1, "Unknown tag of error info value.");
        return error_info.template get<InfoTag>();
    }

    template<string_tag InfoTag>
    datum const &get() const noexcept {
        static_assert(count_tag_if<InfoTags...>(InfoTag) == 1, "Unknown tag of error info value.");
        return error_info.template get<InfoTag>();
    }

    template<string_tag InfoTag>
    bool has() const noexcept {
        static_assert(count_tag_if<InfoTags...>(InfoTag) == 1, "Unknown tag of error info value.");
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
                tag_to_string(error_info.get_tag(i)),
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
using parse_error = sub_error<"parse_error"_tag, "url"_tag, "line"_tag, "column"_tag, "offset"_tag, "previous_msg"_tag, "parse_string"_tag>;

/** Error to throw when an operation can not be executed due to the type of its operants.
** This is for example used in universal_type.
**/
using invalid_operation_error = sub_error<"invalid_op"_tag, "url"_tag, "line"_tag, "column"_tag, "offset"_tag, "previous_msg"_tag>;

using url_error = sub_error<"url_error"_tag, "url"_tag>;
using io_error = sub_error<"io_error"_tag, "url"_tag, "errno"_tag, "error_message"_tag>;
using key_error = sub_error<"key_error"_tag, "key"_tag>;
using gui_error = sub_error<"gui_error"_tag, "vk_result"_tag>;
using bounds_error = sub_error<"bounds_error"_tag>;
using overflow_error = sub_error<"overflow"_tag>;


#define TTAURI_THROW(x) throw std::move((x).log(__FILE__, __LINE__));

#define parse_assert(x) if (!(x)) { TTAURI_THROW(parse_error("{}", #x )); }
#define parse_assert2(x, msg) if (!(x)) { TTAURI_THROW(parse_error(msg)); }

#define hresult_assert_or_throw(x) ([](HRESULT result) {\
        if (ttauri_unlikely(FAILED(result))) {\
            TTAURI_THROW(io_error("Call to '{}' failed with {:08x}", #x, result));\
        }\
        return result;\
    }(x))


}
