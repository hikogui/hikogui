// Copyright Take Vos 2019-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "JSON.hpp"
#include "../file/module.hpp"
#include "../macros.hpp"
#include <filesystem>

namespace hi { inline namespace v1 {
namespace detail {

struct parse_context_t {
    std::filesystem::path path;
    std::string_view::const_iterator text_begin;
};

template<std::input_iterator It, std::sentinel_for<It> ItEnd>
[[nodiscard]] std::optional<datum> json_parse_value(parse_context_t& context, It &it, ItEnd last);

template<std::input_iterator It, std::sentinel_for<It> ItEnd>
[[nodiscard]] inline std::optional<datum> json_parse_array(parse_context_t& context, It &it, ItEnd last)
{
    auto r = datum::make_vector();

    // Required '['
    if (*it == '[') {
        ++it;
    } else {
        return std::nullopt;
    }

    auto comma_after_value = true;
    while (true) {
        // A ']' is required at end of configuration-items.
        if (*it == ']') {
            ++it;
            break;

            // Required a value.
        } else if (auto result = json_parse_value(context, it, last)) {
            if (not comma_after_value) {
                throw parse_error(std::format("{}: Expecting ',', found {}", token_location(it, last, context.path), *it));
            }

            r.push_back(std::move(*result));

            if (*it == ',') {
                ++it;
                comma_after_value = true;
            } else {
                comma_after_value = false;
            }

        } else {
            throw parse_error(std::format("{}: Expecting a JSON value, found {}", token_location(it, last, context.path), *it));
        }
    }

    return r;
}

template<std::input_iterator It, std::sentinel_for<It> ItEnd>
[[nodiscard]] inline std::optional<datum> json_parse_object(parse_context_t& context, It& it, ItEnd last)
{
    auto r = datum::make_map();

    // Required '{'
    if (*it == '{') {
        ++it;

    } else {
        return std::nullopt;
    }

    auto comma_after_value = true;
    while (true) {
        // A '}' is required at end of configuration-items.
        if (*it == '}') {
            ++it;
            break;

            // Required a string name.
        } else if (*it == token::dstr) {
            if (not comma_after_value) {
                throw parse_error(std::format("{}: Expecting ',', found {}.", token_location(it, last, context.path), *it));
            }

            auto name = static_cast<std::string>(*it++);

            if ((*it == ':')) {
                ++it;
            } else {
                throw parse_error(std::format("{}: Expecting ':', found {}.", token_location(it, last, context.path), *it));
            }

            if (auto result = json_parse_value(context, it, last)) {
                r[name] = std::move(*result);

            } else {
                throw parse_error(
                    std::format("{}: Expecting a JSON value, found {}.", token_location(it, last, context.path), *it));
            }

            if (*it == ',') {
                ++it;
                comma_after_value = true;
            } else {
                comma_after_value = false;
            }

        } else {
            throw parse_error(std::format(
                "{}: Unexpected token {}, expected a key or close-brace.", token_location(it, last, context.path), *it));
        }
    }

    return r;
}

template<std::input_iterator It, std::sentinel_for<It> ItEnd>
[[nodiscard]] inline std::optional<datum> json_parse_value(parse_context_t& context, It& it, ItEnd last)
{
    hi_assert(it != last);

    if (*it == token::dstr) {
        return datum{static_cast<std::string>(*it++)};

    } else if (*it == token::integer) {
        return datum{static_cast<long long>(*it++)};

    } else if (*it == token::real) {
        return datum{static_cast<double>(*it++)};

    } else if (*it == '-') {
        ++it;
        if (*it == token::integer) {
            return datum{-static_cast<long long>(*it++)};

        } else if (*it == token::real) {
            return datum{-static_cast<double>(*it++)};

        } else {
            throw parse_error(std::format(
                "{}: Unexpected token '{}' after '-', expected integer or floating point literal.",
                token_location(it, last, context.path),
                *it));
        }

    } else if (*it == token::id and *it == "true") {
        ++it;
        return datum{true};

    } else if (*it == token::id and *it == "false") {
        ++it;
        return datum{false};

    } else if (*it == token::id and *it == "null") {
        ++it;
        return datum{nullptr};

    } else if (auto object = json_parse_object(context, it, last)) {
        return *object;

    } else if (auto array = json_parse_array(context, it, last)) {
        return *array;

    } else {
        return std::nullopt;
    }
}

} // namespace detail

[[nodiscard]] datum parse_JSON(std::string_view text)
{
    auto it = lexer<lexer_config::json_style()>.parse(text);

    auto context = detail::parse_context_t{};
    context.path = "<no-filename>";
    context.text_begin = text.begin();

    if (it == std::default_sentinel) {
        throw parse_error(std::format("{}: No tokens found", token_location(it, std::default_sentinel, context.path)));
    }

    auto r = datum{};
    if (auto result = json_parse_value(context, it, std::default_sentinel)) {
        r = std::move(*result);

    } else {
        throw parse_error(std::format("{}: Missing JSON object", token_location(it, std::default_sentinel, context.path)));
    }

    if (it != std::default_sentinel) {
        throw parse_error(
            std::format("{}: Unexpected text after JSON root object", token_location(it, std::default_sentinel, context.path)));
    }

    return r;
}

static void format_JSON_impl(datum const& value, std::string& result, hi::indent indent = {})
{
    if (holds_alternative<nullptr_t>(value)) {
        result += "null";
    } else if (hilet *b = get_if<bool>(value)) {
        result += *b ? "true" : "false";
    } else if (hilet *i = get_if<long long>(value)) {
        result += hi::to_string(*i);
    } else if (hilet *f = get_if<double>(value)) {
        result += hi::to_string(*f);
    } else if (hilet *s = get_if<std::string>(value)) {
        result += '"';
        for (hilet c : *s) {
            switch (c) {
            case '\n':
                result += '\\';
                result += 'n';
                break;
            case '\r':
                result += '\\';
                result += 'r';
                break;
            case '\t':
                result += '\\';
                result += 't';
                break;
            case '\f':
                result += '\\';
                result += 'f';
                break;
            case '"':
                result += '\\';
                result += '"';
                break;
            default:
                result += c;
            }
        }
        result += '"';

    } else if (hilet *v = get_if<datum::vector_type>(value)) {
        result += indent;
        result += '[';
        result += '\n';

        for (auto it = v->begin(); it != v->end(); it++) {
            if (it != v->begin()) {
                result += ',';
                result += '\n';
            }
            result += indent + 1;

            format_JSON_impl(*it, result, indent + 1);
        }

        result += '\n';
        result += indent;
        result += ']';
    } else if (hilet *m = get_if<datum::map_type>(value)) {
        result += indent;
        result += '{';
        result += '\n';

        for (auto it = m->begin(); it != m->end(); it++) {
            if (it != m->begin()) {
                result += ',';
                result += '\n';
            }
            result += indent + 1;

            format_JSON_impl(it->first, result, indent + 1);
            result += ':';
            result += ' ';
            format_JSON_impl(it->second, result, indent + 1);
        }

        result += '\n';
        result += indent;
        result += '}';
    } else {
        hi_no_default();
    }
}

[[nodiscard]] std::string format_JSON(datum const& root)
{
    auto r = std::string{};
    format_JSON_impl(root, r);
    r += '\n';
    return r;
}

}} // namespace hi::v1
