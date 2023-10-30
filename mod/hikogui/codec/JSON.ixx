// Copyright Take Vos 2019.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"

#include <string>
#include <string_view>
#include <vector>
#include <optional>

export module hikogui_codec_JSON;
import hikogui_algorithm;
import hikogui_codec_datum;
import hikogui_codec_indent;
import hikogui_file;
import hikogui_parser;
import hikogui_utility;

export namespace hi::inline v1 {
namespace detail {

template<std::input_iterator It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr std::optional<datum> json_parse_value(It &it, ItEnd last, std::string_view path);

template<std::input_iterator It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr std::optional<datum> json_parse_array(It &it, ItEnd last, std::string_view path)
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
        } else if (auto result = json_parse_value(it, last, path)) {
            if (not comma_after_value) {
                throw parse_error(std::format("{}: Expecting ',', found {}", token_location(it, last, path), *it));
            }

            r.push_back(std::move(*result));

            if (*it == ',') {
                ++it;
                comma_after_value = true;
            } else {
                comma_after_value = false;
            }

        } else {
            throw parse_error(std::format("{}: Expecting a JSON value, found {}", token_location(it, last, path), *it));
        }
    }

    return r;
}

template<std::input_iterator It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr std::optional<datum> json_parse_object(It& it, ItEnd last, std::string_view path)
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
                throw parse_error(std::format("{}: Expecting ',', found {}.", token_location(it, last, path), *it));
            }

            auto name = static_cast<std::string>(*it++);

            if ((*it == ':')) {
                ++it;
            } else {
                throw parse_error(std::format("{}: Expecting ':', found {}.", token_location(it, last, path), *it));
            }

            if (auto result = json_parse_value(it, last, path)) {
                r[name] = std::move(*result);

            } else {
                throw parse_error(
                    std::format("{}: Expecting a JSON value, found {}.", token_location(it, last, path), *it));
            }

            if (*it == ',') {
                ++it;
                comma_after_value = true;
            } else {
                comma_after_value = false;
            }

        } else {
            throw parse_error(std::format(
                "{}: Unexpected token {}, expected a key or close-brace.", token_location(it, last, path), *it));
        }
    }

    return r;
}

template<std::input_iterator It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr std::optional<datum> json_parse_value(It& it, ItEnd last, std::string_view path)
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
                token_location(it, last, path),
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

    } else if (auto object = json_parse_object(it, last, path)) {
        return *object;

    } else if (auto array = json_parse_array(it, last, path)) {
        return *array;

    } else {
        return std::nullopt;
    }
}

} // namespace detail

export template<std::input_iterator It, std::sentinel_for<It> ItEnd>
[[nodiscard]] constexpr datum parse_JSON(It it, ItEnd last, std::string_view path = std::string_view{"<none>"})
{
    auto token_it = lexer<lexer_config::json_style()>.parse(it, last);

    if (token_it == std::default_sentinel) {
        throw parse_error(std::format("{}: No tokens found", token_location(token_it, std::default_sentinel, path)));
    }

    auto r = datum{};
    if (auto result = json_parse_value(token_it, std::default_sentinel, path)) {
        r = std::move(*result);

    } else {
        throw parse_error(std::format("{}: Missing JSON object", token_location(token_it, std::default_sentinel, path)));
    }

    if (token_it != std::default_sentinel) {
        throw parse_error(
            std::format("{}: Unexpected text after JSON root object", token_location(token_it, std::default_sentinel, path)));
    }

    return r;
}

/** Parse a JSON string.
 * @param text The text to parse.
 * @return A datum representing the parsed object.
 */
export [[nodiscard]] constexpr datum parse_JSON(std::string_view text, std::string_view path = std::string_view{"<none>"})
{
    return parse_JSON(text.cbegin(), text.cend(), path);
}

/** Parse a JSON string.
 * @param text The text to parse.
 * @return A datum representing the parsed object.
 */
export [[nodiscard]] constexpr datum parse_JSON(std::string const& text, std::string_view path = std::string_view{"<none>"})
{
    return parse_JSON(std::string_view{text}, path);
}

/** Parse a JSON string.
 * @param text The text to parse.
 * @return A datum representing the parsed object.
 */
export [[nodiscard]] constexpr datum parse_JSON(char const *text, std::string_view path = std::string_view{"<none>"})
{
    return parse_JSON(std::string_view{text}, path);
}

/** Parse a JSON string.
 * @param path A path pointing to the file to parse.
 * @return A datum representing the parsed object.
 */
export [[nodiscard]] datum parse_JSON(std::filesystem::path const& path)
{
    return parse_JSON(as_string_view(file_view(path)), path.string());
}

export constexpr void format_JSON_impl(datum const& value, std::string& result, hi::indent indent = {})
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

/** Dump an datum object into a JSON string.
 * @param root datum-object to serialize
 * @return The JSON serialized object as a string
 */
export [[nodiscard]] constexpr std::string format_JSON(datum const& root)
{
    auto r = std::string{};
    format_JSON_impl(root, r);
    r += '\n';
    return r;
}

} // namespace hi::inline v1
