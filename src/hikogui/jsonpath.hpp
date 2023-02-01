// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "utility/module.hpp"
#include "tokenizer.hpp"
#include "generator.hpp"
#include <string>
#include <variant>
#include <string_view>
#include <vector>
#include <limits>
#include <format>

namespace hi::inline v1 {

struct jsonpath_root {
    [[nodiscard]] std::string string() const noexcept
    {
        return "$";
    }

    [[nodiscard]] bool is_singular() const noexcept
    {
        return true;
    }
};

struct jsonpath_current {
    [[nodiscard]] std::string string() const noexcept
    {
        return "@";
    }

    [[nodiscard]] bool is_singular() const noexcept
    {
        return true;
    }
};

struct jsonpath_wildcard {
    [[nodiscard]] std::string string() const noexcept
    {
        return "[*]";
    }

    [[nodiscard]] bool is_singular() const noexcept
    {
        return false;
    }
};

struct jsonpath_descend {
    [[nodiscard]] std::string string() const noexcept
    {
        return "..";
    }

    [[nodiscard]] bool is_singular() const noexcept
    {
        return false;
    }
};

struct jsonpath_names {
    std::vector<std::string> names;

    hi_constexpr jsonpath_names(jsonpath_names const &) noexcept = default;
    hi_constexpr jsonpath_names(jsonpath_names &&) noexcept = default;
    hi_constexpr jsonpath_names &operator=(jsonpath_names const &) noexcept = default;
    hi_constexpr jsonpath_names &operator=(jsonpath_names &&) noexcept = default;

    jsonpath_names(std::string other) noexcept : names()
    {
        names.push_back(std::move(other));
    }

    [[nodiscard]] std::string string() const noexcept
    {
        auto r = std::string{"["};
        auto first = true;
        for (hilet &name : names) {
            if (not first) {
                r += ',';
            }

            r += '\'';
            r += name;
            r += '\'';
            first = false;
        }
        r += ']';
        return r;
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return names.size();
    }

    [[nodiscard]] std::string const &front() const noexcept
    {
        return names.front();
    }

    [[nodiscard]] auto begin() const noexcept
    {
        return names.begin();
    }

    [[nodiscard]] auto end() const noexcept
    {
        return names.end();
    }

    void push_back(std::string rhs) noexcept
    {
        names.push_back(std::move(rhs));
    }

    [[nodiscard]] bool is_singular() const noexcept
    {
        return names.size() == 1;
    }
};

struct jsonpath_indices {
    std::vector<ssize_t> indices;

    hi_constexpr jsonpath_indices(jsonpath_indices const &) noexcept = default;
    hi_constexpr jsonpath_indices(jsonpath_indices &&) noexcept = default;
    hi_constexpr jsonpath_indices &operator=(jsonpath_indices const &) noexcept = default;
    hi_constexpr jsonpath_indices &operator=(jsonpath_indices &&) noexcept = default;

    jsonpath_indices(ssize_t other) noexcept : indices()
    {
        indices.push_back(other);
    }

    [[nodiscard]] std::string string() const noexcept
    {
        auto r = std::string{"["};
        auto first = true;
        for (hilet index : indices) {
            if (not first) {
                r += ',';
            }

            r += hi::to_string(index);
            first = false;
        }
        r += ']';
        return r;
    }

    [[nodiscard]] generator<std::size_t> filter(std::size_t size) const noexcept
    {
        hilet size_ = narrow_cast<ssize_t>(size);

        for (hilet index : indices) {
            hilet index_ = index >= 0 ? index : size_ + index;
            if (index_ >= 0 and index_ < size_) {
                co_yield narrow_cast<std::size_t>(index_);
            }
        }
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return indices.size();
    }

    [[nodiscard]] ssize_t const &front() const noexcept
    {
        return indices.front();
    }

    void push_back(ssize_t rhs) noexcept
    {
        indices.push_back(rhs);
    }

    [[nodiscard]] bool is_singular() const noexcept
    {
        return indices.size() == 1;
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

    /** Get the start offset.
     *
     * @param size The size of the container.
     * @return The start offset of the slice.
     */
    [[nodiscard]] std::size_t begin(std::size_t size) const noexcept
    {
        hilet size_ = narrow_cast<ssize_t>(size);
        hilet begin = first >= 0 ? first : size_ + first;
        return narrow_cast<std::size_t>(std::clamp(begin, 0_z, size_));
    }

    /** Get the one-step beyond last offset.
     * This will calculate the last offset of an integer number of steps
     * starting from begin(). This allows the end() to be equality compared inside
     * a for loop, even when the step is negative.
     *
     * @param size The size of the container.
     * @return The one-step beyond last offset of the slice.
     */
    [[nodiscard]] std::size_t end(std::size_t size) const noexcept
    {
        hilet size_ = narrow_cast<ssize_t>(size);
        hilet last_ = std::clamp(
            last == std::numeric_limits<ssize_t>::min() ? size_ :
                last >= 0                               ? last :
                                                          size_ + last,
            0_z,
            size_);

        hilet first_ = begin(size);
        hilet distance = last_ - first_;
        hilet steps = distance / step;
        return narrow_cast<std::size_t>(first_ + steps * step);
    }

    [[nodiscard]] bool last_is_empty() const noexcept
    {
        return last == std::numeric_limits<ssize_t>::min();
    }

    [[nodiscard]] std::string string() const noexcept
    {
        if (last_is_empty()) {
            return std::format("[{}:e:{}]", first, step);
        } else {
            return std::format("[{}:{}:{}]", first, last, step);
        }
    }

    [[nodiscard]] bool is_singular() const noexcept
    {
        return false;
    }
};

// clang-format off
using jsonpath_node = std::variant<
    jsonpath_root, jsonpath_current, jsonpath_wildcard, jsonpath_descend, jsonpath_names, jsonpath_indices, jsonpath_slice>;
// clang-format on

[[nodiscard]] inline jsonpath_node parse_jsonpath_slicing_operator(auto &it, auto it_end, ssize_t first)
{
    ++it;
    auto last = std::numeric_limits<ssize_t>::min();
    if (*it == tokenizer_name_t::IntegerLiteral) {
        last = static_cast<ssize_t>(*it);
        ++it;
    }

    auto step = 1_z;
    if (*it == tokenizer_name_t::Operator and *it == ":") {
        ++it;
        hi_check(*it == tokenizer_name_t::IntegerLiteral, "Expect integer as third slice argument, got {}.", *it);
        step = static_cast<ssize_t>(*it);
        ++it;
    }

    hi_check(*it == tokenizer_name_t::Operator and *it == "]", "Expected end of slicing operator ']', got {}.", *it);

    hi_check(step != 0, "Slicing operator's step must not be zero");
    return jsonpath_slice{first, last, step};
}

[[nodiscard]] inline jsonpath_node parse_jsonpath_integer_indexing_operator(auto &it, auto it_end, ssize_t first)
{
    auto tmp = jsonpath_indices(first);

    while (*it == tokenizer_name_t::Operator and *it == ",") {
        ++it;
        hi_check(*it == tokenizer_name_t::IntegerLiteral, "Expect integer literal after comma ',', got {}.", *it);
        tmp.push_back(static_cast<ssize_t>(*it));
        ++it;
    }

    hi_check(*it == tokenizer_name_t::Operator and *it == "]", "Expected end of slicing operator ']', got {}.", *it);
    return tmp;
}

[[nodiscard]] inline jsonpath_node parse_jsonpath_name_indexing_operator(auto &it, auto it_end, std::string first)
{
    auto tmp = jsonpath_names(std::move(first));

    while (*it == tokenizer_name_t::Operator and *it == ",") {
        ++it;
        hi_check(*it == tokenizer_name_t::StringLiteral, "Expect string literal after comma ',', got {}.", *it);
        tmp.push_back(static_cast<std::string>(*it));
        ++it;
    }

    hi_check(*it == tokenizer_name_t::Operator and *it == "]", "Expected end of indexing operator ']', got {}.", *it);
    return tmp;
}

[[nodiscard]] inline jsonpath_node parse_jsonpath_indexing_operator(auto &it, auto it_end)
{
    ++it;

    if (*it == tokenizer_name_t::Operator and *it == "*") {
        ++it;
        hi_check(*it == tokenizer_name_t::Operator and *it == "]", "Expected end of indexing operator ']', got {}.", *it);
        return jsonpath_wildcard{};

    } else if (*it == tokenizer_name_t::Operator and *it == ":") {
        return parse_jsonpath_slicing_operator(it, it_end, 0);

    } else if (*it == tokenizer_name_t::IntegerLiteral) {
        hilet first = static_cast<ssize_t>(*it);

        ++it;
        if (*it == tokenizer_name_t::Operator and *it == ":") {
            return parse_jsonpath_slicing_operator(it, it_end, first);
        } else {
            return parse_jsonpath_integer_indexing_operator(it, it_end, first);
        }

    } else if (*it == tokenizer_name_t::StringLiteral) {
        auto first = static_cast<std::string>(*it);

        ++it;
        return parse_jsonpath_name_indexing_operator(it, it_end, first);

    } else {
        throw parse_error(std::format("Expected a integer index or child name after indexing operator '[', got token {}.", *it));
    }
}

[[nodiscard]] inline jsonpath_node parse_jsonpath_child_operator(auto &it, auto it_end)
{
    ++it;

    if (*it == tokenizer_name_t::Operator and *it == "*") {
        return jsonpath_wildcard{};

    } else if (*it == tokenizer_name_t::Operator and *it == ".") {
        if (*(it + 1) == tokenizer_name_t::Operator and *(it + 1) == "[") {
            // When the descend operator '..' is followed by an indexing operator.
            // Then the full descend operator is consumed here.
            return jsonpath_descend{};

        } else {
            // The descend operator '..' is often followed by a name or '*' as-if the
            // second dot in the descend operator is a child selector. Rewind so that
            // the parser will use the second dot as a child selector.
            --it;
            return jsonpath_descend{};
        }

    } else if (*it == tokenizer_name_t::Name) {
        return jsonpath_names{static_cast<std::string>(*it)};

    } else {
        throw parse_error(std::format("Expected a child name or wildcard, got token {}.", *it));
    }
}

class jsonpath {
public:
    using container_type = std::vector<jsonpath_node>;
    using value_type = typename container_type::value_type;
    using iterator = typename container_type::iterator;
    using const_iterator = typename container_type::const_iterator;

    [[nodiscard]] jsonpath(std::string_view rhs) : _nodes()
    {
        auto tokens = parseTokens(rhs);
        hilet it_end = tokens.cend();
        for (auto it = tokens.cbegin(); it != it_end; ++it) {
            if (*it == tokenizer_name_t::Operator and *it == ".") {
                _nodes.emplace_back(parse_jsonpath_child_operator(it, it_end));

            } else if (*it == tokenizer_name_t::Operator and *it == "[") {
                _nodes.emplace_back(parse_jsonpath_indexing_operator(it, it_end));

            } else if (*it == tokenizer_name_t::Name and *it == "$") {
                hi_check(_nodes.empty(), "Root node '$' not at start of path.");
                _nodes.emplace_back(jsonpath_root{});

            } else if (*it == tokenizer_name_t::Operator and *it == "@") {
                hi_check(_nodes.empty(), "Current node '@' not at start of path.");
                _nodes.emplace_back(jsonpath_current{});

            } else if (*it == tokenizer_name_t::Name) {
                hi_check(_nodes.empty(), "Unexpected child name {}.", *it);
                _nodes.emplace_back(jsonpath_names{static_cast<std::string>(*it)});

            } else if (*it == tokenizer_name_t::End) {
                continue;

            } else {
                throw parse_error(std::format("Unexpected token {}.", *it));
            }
        }
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return _nodes.empty();
    }

    /** The json-path will result in zero or one match.
     */
    [[nodiscard]] bool is_singular() const noexcept
    {
        auto r = true;
        for (hilet &node : _nodes) {
            r &= std::visit(
                [](hilet &node_) {
                    return node_.is_singular();
                },
                node);
        }
        return r;
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return _nodes.size();
    }

    [[nodiscard]] iterator begin() noexcept
    {
        return _nodes.begin();
    }

    [[nodiscard]] const_iterator begin() const noexcept
    {
        return _nodes.begin();
    }

    [[nodiscard]] const_iterator cbegin() const noexcept
    {
        return _nodes.cbegin();
    }

    [[nodiscard]] iterator end() noexcept
    {
        return _nodes.end();
    }

    [[nodiscard]] const_iterator end() const noexcept
    {
        return _nodes.end();
    }

    [[nodiscard]] const_iterator cend() const noexcept
    {
        return _nodes.cend();
    }

    [[nodiscard]] friend std::string to_string(jsonpath const &path) noexcept
    {
        auto r = std::string{};
        for (hilet &node : path._nodes) {
            r += std::visit(
                [](hilet &node_) {
                    return node_.string();
                },
                node);
        }
        return r;
    }

private:
    std::vector<jsonpath_node> _nodes;
};

} // namespace hi::inline v1

template<typename CharT>
struct std::formatter<hi::jsonpath, CharT> : std::formatter<char const *, CharT> {
    auto format(hi::jsonpath const &t, auto &fc)
    {
        return std::formatter<std::string, CharT>{}.format(to_string(t), fc);
    }
};
