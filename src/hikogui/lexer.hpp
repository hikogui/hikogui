// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "unicode/unicode_identifier.hpp"
#include "char_maps/utf_8.hpp"

namespace hi { inline namespace v1 {

template<typename It, typename EndIt>
class lexer {
public:
    class token_type {
    private:
        // id,string-literal,
        std::variant<std::string, std::string, size_t> _v;
    };

    class const_iterator {
    public:
        constexpr const_iterator(lexer *lexer) noexcept : _lexer(lexer), _it(lexer->_begin), _end(lexer->_end)
        {
            parse_token();
        }

        constexpr const_iterator& operator++() const noexcept
        {
            parse_token();
            return *this;
        }

        [[nodiscard]] constexpr bool operator==(std::default_sentinel_t) const noexcept
        {
            return _finished
        }

    private:
        It _it;
        EndIt _end;
        char _number_group_separator = ',';
        size_t _tab_size = 8;

        lexer *_lexer = nullptr;
        bool _finished = false;
        std::string _buffer = {};

        [[nodiscard]] constexpr std::optional<token_type> parse_string() noexcept
        {

        }

        [[nodiscard]] constexpr std::optional<token_type> parse_id() noexcept
        {
            hi_axiom(_it != end);

            hilet first = _it;

            // The first character of the ID does not include digits.
            if (hilet[c, good] = char_map<"utf-8">{}.read(_it, _end); not is_ID_Start(c)) {
                // This was not a ID character, rewind.
                _it = first;
                return std::nullopt;
            }

            while (_it != _end) {
                hilet prev_it = _it;
                if (hilet[c, good] = char_map<"utf-8">{}.read(_it, _end); not is_ID_Continue(c)) {
                    // This was not a ID character, rewind this last character.
                    _it = prev_it;
                    return token_type::id(first, _it);
                }
            }

            // ID at end-of-text.
            return token_type::id(first, _it);
        }

        [[nodiscard]] constexpr std::optional<token_type> parse_number() noexcept
        {
            enum class state_type { integer, floating_point, now_exponent, has_exponent };

            hi_axiom(_it != _end);
            hilet first = it;
            float_state state = state_type::integer;

            // Check first character of number.
            if (*_it == '.') {
                state = state_type::floating_point;
            } else if (not(*_it >= '0' and *_it <= '9')) {
                return std::nullopt;
            }

            // Reuse the allocation of _buffer.
            _buffer.clear();
            _buffer += *it;

            // Check the continues characters of a number.
            bool is_hex = false;
            for (++_it; it != _end; ++_it) {
                hilet c = *_it;
                if ((c >= '0' and c <= '9')) {
                    _buffer += c;
                    state = (state == state_type::now_exponent) ? state_type::has_exponent : state;

                } else if (((c >= 'a' and c <= 'f') or (c >= 'A' and c <= 'F')) and is_hex) {
                    // The letters 'a' to 'f' or 'A' to 'F' may only appear in hexadecimal numbers.
                    _buffer += c;
                    state = (state == state_type::now_exponent) ? state_type::has_exponent : state;

                } else if (c == _number_group_separator) {
                    // Don't add the number-group-separator to the buffer, which `from_chars()` can't parse.
                    state = (state == state_type::now_exponent) ? state_type::has_exponent : state;

                } else if (c == '.' and state < state_type::floating_point) {
                    // A single '.' will make this a floating-point number, and can appear at most once.
                    _buffer += c;
                    state = state_type::floating_point;

                } else if ((c == 'x' or c == 'X') and _buffer == "0") {
                    // A hexadecimal marker may only appear directly after the first digit when that digit is '0'.
                    _buffer += c;
                    is_hex = true;

                } else if ((c == 'b' or c == 'b' or c == 'o' or c == 'O' or c == 'd' or c == 'D') and _buffer == "0") {
                    // A binary/octal/decimal marker may only appear directly after the first digit when that digit is '0'.
                    _buffer += c;

                } else if ((c == 'e' or c == 'E') and state < state_type::now_exponent) {
                    // A single 'e' or 'E' will make this a floating-point number with an exponent, and can appear at most once.
                    _buffer += c;
                    state = state_type::now_exponent;

                } else if ((c == '-' or c == '+') and state == state_type::now_exponent) {
                    // A '+' or '-' may only appear directly after the exponent marker 'e' or 'E'.
                    _buffer += c;
                    state = state_type::has_exponent;

                } else if (state >= state_type::floating_point) {
                    // end-of-number.
                    if (_buffer == ".") {
                        // Rewind and reinterpret as an operator.
                        _it = first;
                        return std::nullopt;
                    } else {
                        // Floating-point with an optional suffix.
                        return token_type::floating_point{first, _buffer, parse_id()};
                    }

                } else {
                    // Integer with an optional suffix.
                    return token_type::integer{first, _buffer, parse_id()};
                }
            }

            // The number was at end-of-text.
            if (state >= state_type::floating_point) {
                if (_buffer == ".") {
                    // Rewind and reinterpret as an operator.
                    _it = first;
                    return std::nullopt;
                } else {
                    return token_type::floating_point{first, _buffer};
                }
            } else {
                return token_type::integer{first, _buffer};
            }
        }

        [[nodiscard]] constexpr token_type parse_token()
        {
            while (_it != _end) {
                // For performance try ASCII first.
                if (parse_white_space()) {
                    continue; // Skip over whitespace

                } else if (auto syntax_token = parse_syntax()) {
                    return *syntax_token;

                } else if (auto number_token = parse_number()) {
                    return *number_token;

                } else if (auto string_token = parse_string()) {
                    return *string_token;

                } else if (auto comment_token = parse_comment()) {
                    return *comment_token;

                } else if (auto id_token = parse_id()) {
                    return *id_token;

                } else {
                    hilet[c, good] = char_set<"utf-8">{}.read(_it, _end);
                    throw parse_error(_begin, _it, _tab_size, "Unexpected character U+{:04x}.", char_cast<uint32_t>(c32));
                }
            }

            // Empty token is end-of-text.
            return {};
        }
    };

    constexpr lexer(It first, EndIt last) noexcept : _first(first), _last(last) {}

    [[nodiscard]] constexpr const_iterator begin() const noexcept
    {
        return const_iterator{this};
    }

    [[nodiscard]] constexpr std::default_sentinel_t end() const noexcept
    {
        return {};
    }

private:
    It _begin;
    EndIt _end;
};
}} // namespace hi::v1
