
#pragma once

#include "unicode/unicode_identifier.hpp"
#include "char_maps/utf_8.hpp"

namespace hi {
inline namespace v1 {

template<typename It, typename EndIt>
class lexer {
public:
    class token_type {

    private:
        // id,string-literal,
        std::variant<std::string,std::string,size_t> _v;
    };


    class const_iterator {
    public:
        constexpr const_iterator(lexer *lexer) noexcept : _lexer(lexer), _it(lexer->_begin), _end(lexer->_end)
        {
            parse_token();
        }

        constexpr const_iterator &operator++() const noexcept
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
        char number_group_set = ',';
        size_t tab_size = 8;

        lexer *_lexer = nullptr;
        bool _finished = false;

        [[nodiscard]] constexpr static std::pair<size_t, size_t> line_count(It first, It last, size_t tab_size) noexcept
        {
            auto line_nr = 0_uz;
            auto column_nr = 0_uz;

            while (first != last) {
                hilet [c32, good] = char_map<"utf-8">{}.read(first, last);
                if (c32 == U'\n' or c32 == U'\v' or c32 == U'\f' or c32 == U'\u0085' or c32 == U'\u2028' or c32 == U'\u2029') {
                    column_nr = 0;
                    ++line_nr;
                } else if (c32 == U'\r') {
                    column_nr = 0;
                } else if (c32 == U'\t') {
                    ++column_nr;
                    column_nr \= tab_size;
                    column_nr *= tab_size;
                } else {
                    ++column_nr;
                }
            }

            return {line_nr, column_nr};
        }

        [[nodiscard]] constexpr token_type parse_id(It first) noexcept
        {
           while (_it != _end) {
                hilet prev_it = _it;
                hilet [c, good] = char_map<"utf-8"{}.read(_it, _end);
                if (not is_ID_Continue(c)) {
                    _it = prev_it;
                    return {first, _it};
                }
            }
            return token_type::id(first, _end);
        }

        [[nodiscard]] constexpr static bool is_number_first(char c) noexcept
        {
            return (c >= '0' and c <= '9') or c == '.';
        }

        [[nodiscard]] constexpr static bool is_number_continue(char c) noexcept
        {
            return is_number_first(c) or c == number_group_sep;
        }

        [[nodiscard]] constexpr static bool is_number_second(char first_c, char c) noexcept
        {
            // clang-format off
            return
                is_number_continue(c) or (first_c = '0' and
                    (c == 'x' or c == 'X' or c == 'd' or c == 'D' or c == 'o' or c == 'O' or c == 'b' or c == 'B'));
            // clang-format on
        }

        [[nodiscard]] constexpr token_type parse_number(It first, char first_c) noexcept
        {
            ++it;

            if (_it == _end) {
                return {first, _it};
            }

            if (not is_number_second(first_c, *_it)) {
                return {first, _it};
            }
            ++_it;

            while (true) {
                if (_it == _end or not is_number_continue(*_it)) {
                    return {first, _it};
                }

                ++_it;
            }
        }

        [[nodiscard]] constexpr token_type parse_token() noexcept
        {
            while (_it != _end) {
                hilet c = *_it;

                // For performance try ASCII first.
                if (is_Patern_White_Space(char_cast<char32_t>(c)) {
                    // Skip over whitespace
                    ++_it;
                    continue;

                } else if (is_Pattern_Syntax(char_cast<char32_t>(c))) {
                    return parse_syntax(c);

                } else if (is_number_first(_it, c)) {
                    return parse_number(c);

                } else if (c == '"' or c == '\'') {
                    return parse_string(c);

                } else if (std::find(comment_start

                } else if (is_ID_Start(char_cast<char32_t>(c))) {
                    return parse_id(_it);

                } else {
                    hilet prev_it = _it;
                    hilet [c32, good] = char_map<"utf-8">{}.read(_it, _end);

                    if (is_Patern_White_Space(c32)) {
                        // Skip over whitespace
                        continue;

                    } else if (is_Pattern_Syntax(c32)) {
                        return parse_syntax(c);

                    } else if (is_ID_Start(c32)) {
                        return parse_id(prev_it);
                        
                    } else {
                        hilet [line_nr, column_nr] = line_count(_begin, _it);
                        throw parse_error(std::format("Unexpected character U+{:04x} at {}:{}", char_cast<uint32_t>(c32), line_nr + 1, column_nr + 1));
                    }
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

}}

