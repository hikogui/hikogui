// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "required.hpp"
#include "tokenizer.hpp"
#include <string>
#include <variant>
#include <string_view>
#include <vector>

namespace tt {

struct jsonpath_root {
};

struct jsonpath_current {
};

struct jsonpath_wildcard {
};

struct jsonpath_descent {
};

struct jsonpath_names {
    std::vector<std::string> names;

    constexpr jsonpath_names(jsonpath_names const &) noexcept = default;
    constexpr jsonpath_names(jsonpath_names &&) noexcept = default;
    constexpr jsonpath_names &operator=(jsonpath_names const &) noexcept = default;
    constexpr jsonpath_names &operator=(jsonpath_names &&) noexcept = default;

    jsonpath_names(std::string other) noexcept : names()
    {
        names.push_back(std::move(other));
    }

    [[nodiscard]] auto begin() const noexcept
    {
        return std::begin(names);
    }

    [[nodiscard]] auto end() const noexcept
    {
        return std::end(names);
    }

    void push_back(std::string rhs) noexcept
    {
        names.push_back(std::move(rhs));
    }
};

struct jsonpath_indices {
    std::vector<ssize_t> indices;

    constexpr jsonpath_indices(jsonpath_indices const &) noexcept = default;
    constexpr jsonpath_indices(jsonpath_indices &&) noexcept = default;
    constexpr jsonpath_indices &operator=(jsonpath_indices const &) noexcept = default;
    constexpr jsonpath_indices &operator=(jsonpath_indices &&) noexcept = default;

    jsonpath_indices(ssize_t other) noexcept : indices()
    {
        indices.push_back(other);
    }

    [[nodiscard]] auto begin() const noexcept
    {
        return std::begin(indices);
    }

    [[nodiscard]] auto end() const noexcept
    {
        return std::end(indices);
    }

    void push_back(ssize_t rhs) noexcept
    {
        indices.push_back(rhs);
    }
};

struct jsonpath_slice {
    ssize_t first;
    ssize_t last;
    ssize_t step;

    constexpr jsonpath_slice(jsonpath_slice const &) noexcept = default;
    constexpr jsonpath_slice(jsonpath_slice &&) noexcept = default;
    constexpr jsonpath_slice &operator=(jsonpath_slice const &) noexcept = default;
    constexpr jsonpath_slice &operator=(jsonpath_slice &&) noexcept = default;

    constexpr jsonpath_slice(ssize_t first, ssize_t last, ssize_t step) noexcept : first(first), last(last), step(step) {}
};

// clang-format off
using jsonpath_node = std::variant<
    jsonpath_root, jsonpath_current, jsonpath_wildcard, jsonpath_descent, jsonpath_names, jsonpath_indices, jsonpath_slice>;
// clang-format on
using jsonpath = std::vector<jsonpath_node>;

[[nodiscard]] inline jsonpath_node parse_jsonpath_slicing_operator(auto &it, auto it_end, ssize_t first)
{
    tt_parse_check(++it != it_end, "Unexpected end of json-path after colon at slicing operator.");
    tt_parse_check(*it == tokenizer_name_t::IntegerLiteral, "Expect integer as second slice argument, got {}.", *it);
    auto last = static_cast<ssize_t>(*it);
    auto step = 1_z;
    tt_parse_check(++it != it_end, "Unexpected end of json-path after second slice argument.");

    if (*it == tokenizer_name_t::Operator and *it == ":") {
        tt_parse_check(++it != it_end, "Unexpected end of json-path after second colon at slicing operator.");
        tt_parse_check(*it == tokenizer_name_t::IntegerLiteral, "Expect integer as third slice argument, got {}.", *it);
        step = static_cast<ssize_t>(*it);
        tt_parse_check(++it != it_end, "Unexpected end of json-path after third slice argument.");
    }

    tt_parse_check(*it == tokenizer_name_t::Operator and *it == "]", "Expected end of slicing operator ']', got {}.", *it);

    tt_parse_check(step != 0, "Slicing operator's step must not be zero");
    return jsonpath_slice{first, last, step};
}

[[nodiscard]] inline jsonpath_node parse_jsonpath_integer_indexing_operator(auto &it, auto it_end, ssize_t first)
{
    auto tmp = jsonpath_indices(first);

    while (*it == tokenizer_name_t::Operator and *it == ",") {
        tt_parse_check(++it != it_end, "Unexpected end of json-path after comma ','.");
        tmp.push_back(static_cast<ssize_t>(*it));
        tt_parse_check(++it != it_end, "Unexpected end of json-path after name at indexing operator '['.");
    }

    tt_parse_check(*it == tokenizer_name_t::Operator and *it == "]", "Expected end of slicing operator ']', got {}.", *it);
    return tmp;
}

[[nodiscard]] inline jsonpath_node parse_jsonpath_name_indexing_operator(auto &it, auto it_end, std::string first)
{
    auto tmp = jsonpath_names(std::move(first));

    while (*it == tokenizer_name_t::Operator and *it == ",") {
        tt_parse_check(++it != it_end, "Unexpected end of json-path after comma ','.");
        tmp.push_back(static_cast<std::string>(*it));
        tt_parse_check(++it != it_end, "Unexpected end of json-path after name at indexing operator '['.");
    }

    tt_parse_check(*it == tokenizer_name_t::Operator and *it == "]", "Expected end of indexing operator ']', got {}.", *it);
    return tmp;
}

[[nodiscard]] inline jsonpath_node parse_jsonpath_indexing_operator(auto &it, auto it_end)
{
    tt_parse_check(++it != it_end, "Unexpected end of json-path at indexing operator '['.");

    if (*it == tokenizer_name_t::Operator and *it == "*") {
        return jsonpath_wildcard{};

    } else if (*it == tokenizer_name_t::IntegerLiteral) {
        auto first = static_cast<ssize_t>(*it);
        tt_parse_check(++it != it_end, "Unexpected end of json-path after index at indexing operator '['.");

        if (*it == tokenizer_name_t::Operator and *it == ":") {
            return parse_jsonpath_slicing_operator(it, it_end, first);
        } else {
            return parse_jsonpath_integer_indexing_operator(it, it_end, first);
        }

    } else if (*it == tokenizer_name_t::StringLiteral) {
        auto first = static_cast<std::string>(*it);
        tt_parse_check(++it != it_end, "Unexpected end of json-path after name at indexing operator '['.");
        return parse_jsonpath_name_indexing_operator(it, it_end, first);

    } else {
        throw parse_error("Expected a integer index or child name after indexing operator '[', got token {}.", *it);
    }
}

[[nodiscard]] inline jsonpath_node parse_jsonpath_child_operator(auto &it, auto it_end)
{
    if (*it == tokenizer_name_t::Operator and *it == "*") {
        return jsonpath_wildcard{};

    } else if (*it == tokenizer_name_t::Name) {
        return jsonpath_names{static_cast<std::string>(*it)};

    } else {
        throw parse_error("Expected a child name or wildcard, got token {}.", *it);
    }
}

[[nodiscard]] inline jsonpath parse_jsonpath(std::string_view rhs)
{
    auto r = jsonpath{};

    auto tokens = parseTokens(rhs);
    ttlet it_end = std::cend(tokens);
    for (auto it = std::cbegin(tokens); it != it_end; ++it) {
        if (*it == tokenizer_name_t::Operator and *it == ".") {
            r.emplace_back(parse_jsonpath_child_operator(it, it_end));

        } else if (*it == tokenizer_name_t::Operator and *it == "[") {
            r.emplace_back(parse_jsonpath_indexing_operator(it, it_end));

        } else if (*it == tokenizer_name_t::Operator and *it == "$") {
            tt_parse_check(r.empty(), "Root node '$' not at start of path.");
            r.emplace_back(jsonpath_root{});

        } else if (*it == tokenizer_name_t::Operator and *it == "@") {
            tt_parse_check(r.empty(), "Current node '@' not at start of path.");
            r.emplace_back(jsonpath_current{});

        } else if (*it == tokenizer_name_t::Name) {
            tt_parse_check(r.empty(), "Unexpected child name {}.", *it);
            r.emplace_back(jsonpath_names{static_cast<std::string>(*it)});

        } else {
            throw parse_error("Unexpected token {}.", *it);
        }
    }

    return r;
}

} // namespace tt
