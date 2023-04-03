// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "token.hpp"
#include "../utility/module.hpp"
#include "../unicode/module.hpp"
#include "../char_maps/module.hpp"
#include <ranges>
#include <iterator>
#include <cstdint>
#include <string>
#include <string_view>
#include <format>
#include <ostream>

namespace hi { inline namespace v1 {

struct lexer_config {
    /** A zero starts in octal number.
     *
     * By default a zero starts a decimal number, but some languages
     * like C and C++ start an octal number with zero.
     */
    uint16_t zero_starts_octal : 1 = 0;

    /** Escaping quotes within a string may be done using quote doubling.
     */
    uint16_t escape_by_quote_doubling : 1 = 0;

    /** The language has a literal color.
     *
     * This is a hash '#' followed by a hexadecimal number.
     */
    uint16_t has_color_literal : 1 = 0;

    uint16_t has_double_quote_string_literal : 1 = 0;
    uint16_t has_single_quote_string_literal : 1 = 0;
    uint16_t has_back_quote_string_literal : 1 = 0;

    uint16_t has_double_slash_line_comment : 1 = 0;
    uint16_t has_hash_line_comment : 1 = 0;
    uint16_t has_semicolon_line_comment : 1 = 0;

    uint16_t has_c_block_comment : 1 = 0;
    uint16_t has_sgml_block_comment : 1 = 0;
    uint16_t filter_white_space : 1 = 0;
    uint16_t filter_comment : 1 = 0;

    /** The equal '=' character is used for INI-like assignment.S
     *
     * After the equal sign '=':
     * - Skip over any non-linefeed whitespace.
     * - If the next character is a annex 31 starter, then the rest of the line is treated as
     *   a string token.
     * - Any other character will resolved as normal.
     */
    uint16_t equal_is_ini_assignment : 1 = 0;

    /** The colon ':' character is used for INI-like assignment.
     *
     * After the colon ':':
     * - Skip over any non-linefeed whitespace.
     * - If the next character is a annex 31 starter, then the rest of the line is treated as
     *   a string token.
     * - Any other character will resolved as normal.
     *
     */
    uint16_t colon_is_ini_assignment : 1 = 0;

    /** The character used to separate groups of numbers.
     *
     * This character is the character that will be ignored by a language
     * if it appears in a integer or floating point literal.
     *
     * For C and C++ this is the quote character, some other languages use
     * an underscore. If the language does not support group separator set this to '\0'.
     */
    char digit_separator = '\0';

    [[nodiscard]] constexpr static lexer_config c_style() noexcept
    {
        auto r = lexer_config{};
        r.filter_white_space = 1;
        r.zero_starts_octal = 1;
        r.digit_separator = '\'';
        r.has_double_quote_string_literal = 1;
        r.has_single_quote_string_literal = 1;
        r.has_double_slash_line_comment = 1;
        r.has_c_block_comment = 1;
        return r;
    }

    [[nodiscard]] constexpr static lexer_config ini_style() noexcept
    {
        auto r = lexer_config{};
        r.filter_white_space = 1;
        r.digit_separator = '_';
        r.has_double_quote_string_literal = 1;
        r.has_single_quote_string_literal = 1;
        r.has_semicolon_line_comment = 1;
        r.has_color_literal = 1;
        r.equal_is_ini_assignment = 1;
        return r;
    }
};

namespace detail {

/** A configurable lexical analyzer with unicode Annex #31 support.
 */
template<lexer_config Config>
class lexer {
private:
    enum class state_type : uint8_t {
        idle,
        zero,
        bin_integer,
        oct_integer,
        dec_integer,
        hex_integer,
        dec_float,
        hex_float,
        dec_sign_exponent,
        hex_sign_exponent,
        dec_exponent,
        hex_exponent,
        dec_exponent_more,
        hex_exponent_more,
        color_literal,
        sqstring_literal,
        sqstring_literal_quote,
        sqstring_literal_escape,
        dqstring_literal,
        dqstring_literal_quote,
        dqstring_literal_escape,
        bqstring_literal,
        bqstring_literal_quote,
        bqstring_literal_escape,
        line_comment,
        block_comment,
        block_comment_found_star,
        block_comment_found_dash,
        block_comment_found_dash_dash,
        block_comment_found_dash_dash_fin0,
        found_colon,
        found_dot,
        found_eq,
        found_hash,
        found_lt,
        found_lt_bang,
        found_lt_bang_dash,
        found_lt_eq,
        found_slash,
        ini_string,
        white_space,
        identifier,

        _size
    };

    /** This is the command to execute for a given state and given character.
     */
    struct command_type {
        /** The state to switch to.
         */
        state_type next_state = state_type::idle;

        /** The token to emit.
         */
        token::kind_type emit_token = token::none;

        /** The char to capture.
         */
        char char_to_capture = '\0';

        /** Clear the capture buffer.
         */
        uint8_t clear : 1 = 0;

        /** Advance the iterator.
         */
        uint8_t advance : 1 = 0;

        /** This entry has been assigned.
         */
        uint8_t assigned : 1 = 0;

        /** Advance line_nr.
         */
        uint8_t advance_line : 1 = 0;

        /** Advance column_nr for a tab.
         */
        uint8_t advance_tab : 1 = 0;
    };

    struct clear_tag {};
    struct any_tag {};
    struct advance_tag {};
    struct capture_tag {};

    class excluding_tag {
    public:
        constexpr excluding_tag(std::string exclusions) noexcept : _exclusions(std::move(exclusions)) {}

        [[nodiscard]] constexpr bool contains(char c) const noexcept
        {
            return _exclusions.find(c) != _exclusions.npos;
        }

    private:
        std::string _exclusions;
    };

    /** Capture a character in the capture buffer.
     */
    constexpr static auto capture = capture_tag{};

    /** Advance the input iterator.
     */
    constexpr static auto advance = advance_tag{};

    /** Clear the capture buffer.
     */
    constexpr static auto clear = clear_tag{};

    /** Match any character.
     */
    constexpr static auto any = any_tag{};

    template<size_t N>
    [[nodiscard]] constexpr excluding_tag excluding(char const (&exclusions)[N]) noexcept
    {
        return excluding_tag{std::string(exclusions, N - 1)};
    }

    template<typename First, typename... Args>
    [[nodiscard]] constexpr static bool _has_advance_tag_argument() noexcept
    {
        if constexpr (std::is_same_v<First, advance_tag>) {
            return true;
        } else if constexpr (sizeof...(Args) == 0) {
            return false;
        } else {
            return _has_advance_tag_argument<Args...>();
        }
    }

    template<typename... Args>
    [[nodiscard]] constexpr static bool has_advance_tag_argument() noexcept
    {
        if constexpr (sizeof...(Args) == 0) {
            return false;
        } else {
            return _has_advance_tag_argument<Args...>();
        }
    }

public:
    constexpr lexer() noexcept : _transition_table()
    {
        using enum state_type;

        add(idle, '/', found_slash, advance, capture);
        add(idle, '<', found_lt, advance, capture);
        add(idle, '#', found_hash, advance, capture);
        add(idle, '.', found_dot, advance, capture);
        add(idle, '=', found_eq, advance, capture);
        add(idle, ':', found_colon, advance, capture);

        add(found_slash, any, idle, token::other);
        add(found_lt, any, idle, token::other);
        add(found_hash, any, idle, token::other);
        add(found_dot, any, idle, token::other);
        add(found_eq, any, idle, token::other);
        add(found_colon, any, idle, token::other);

        // Adds the starters "\"'`"
        add_string_literals();

        // Adds the starters "0123456789"
        add_number_literals();

        add_color_literal();
        add_comments();
        add_white_space();
        add_identifier();
        add_ini_assignment();

        add(idle, "~!@$%^&*()-+[]{}\\|,>?", idle, token::other, capture, advance);

        // All unused entries of the idle state are unexpected characters.
        for (uint8_t i = 0; i != 128; ++i) {
            auto& command = get_command(idle, char_cast<char>(i));
            if (not command.assigned) {
                command.assigned = 1;
                command.advance = 1;
                // If there are actual null characters in the string then nothing gets captured.
                command.char_to_capture = char_cast<char>(i);
                command.emit_token = token::error_unexepected_character;
                command.next_state = idle;
            }
        }
    }

    [[nodiscard]] constexpr command_type& get_command(state_type from, char c) noexcept
    {
        return _transition_table[to_underlying(from) * 128_uz + char_cast<size_t>(c)];
    }

    [[nodiscard]] constexpr command_type const& get_command(state_type from, char c) const noexcept
    {
        return _transition_table[to_underlying(from) * 128_uz + char_cast<size_t>(c)];
    }

    template<typename It, std::sentinel_for<It> ItEnd>
    struct iterator {
    public:
        constexpr iterator(lexer const *lexer, It first, ItEnd last) noexcept :
            _lexer(lexer), _first(first), _last(last), _it(first)
        {
            _cp = advance();
            do {
                _token.kind = parse_token();
            } while (Config.filter_white_space and _token.kind == token::ws);
        }

        [[nodiscard]] constexpr token const& operator*() const noexcept
        {
            return _token;
        }

        constexpr iterator& operator++() noexcept
        {
            hi_axiom(*this != std::default_sentinel);
            do {
                _token.kind = parse_token();
            } while (Config.filter_white_space and _token.kind == token::ws);
            return *this;
        }

        [[nodiscard]] constexpr bool operator==(std::default_sentinel_t) const noexcept
        {
            return _token.kind == token::none;
        }

    private:
        lexer const *_lexer;
        It _first;
        ItEnd _last;
        It _it;
        char32_t _cp = 0;
        token _token;
        state_type _state = state_type::idle;
        size_t _line_nr = 0;
        size_t _column_nr = 0;

        /** Clear the capture buffer..
         *
         * @param code_point The code-point with extra information.
         */
        constexpr void clear() noexcept
        {
            _token.capture.clear();
        }

        /** Write a code point into the capture buffer.
         *
         * @param code_point The code-point.
         */
        constexpr void capture(char code_point) noexcept
        {
            _token.capture.push_back(code_point);
        }

        /** Write a code point into the capture.
         *
         * @param code_point The code-point.
         */
        constexpr void capture(char32_t code_point) noexcept
        {
            hi_axiom(code_point < 0x7fff'ffff);

            auto out_it = std::back_inserter(_token.capture);
            char_map<"utf-8">{}.write(code_point, out_it);
        }

        constexpr void advance_counters() noexcept
        {
            if (_cp == '\n' or _cp == '\v' or _cp == '\f' or _cp == '\x85' or _cp == U'\u2028' or _cp == U'\u2029') {
                ++_line_nr;
            } else if (_cp == '\t') {
                _column_nr /= 8;
                ++_column_nr;
                _column_nr *= 8;
            } else {
                ++_column_nr;
            }
        }

        /** Advances the iterator by a code-point.
         *
         * @return A code-point, or -1 at end of file.
         */
        [[nodiscard]] constexpr char32_t advance() noexcept
        {
            if (_it == _last) {
                return 0xffff'ffff;
            }

            hilet[code_point, valid] = char_map<"utf-8">{}.read(_it, _last);
            return code_point;
        }

        [[nodiscard]] constexpr token::kind_type parse_token_unicode_identifier() noexcept
        {
            switch (ucd_get_lexical_class(_cp & 0x1f'ffff)) {
            case unicode_lexical_class::id_start:
            case unicode_lexical_class::id_continue:
                capture(_cp);
                advance_counters();
                _cp = advance();
                return token::none;

            default:
                _state = state_type::idle;
                return token::id;
            }
        }

        [[nodiscard]] constexpr token::kind_type parse_token_unicode_line_comment() noexcept
        {
            hilet cp_ = _cp & 0x1f'ffff;
            if (cp_ == U'\u0085' or cp_ == U'\u2028' or cp_ == U'\u2029') {
                _state = state_type::idle;
                advance_counters();
                _cp = advance();
                return token::lcomment;

            } else {
                capture(_cp);
                advance_counters();
                _cp = advance();
                return token::none;
            }
        }

        [[nodiscard]] constexpr token::kind_type parse_token_unicode_white_space() noexcept
        {
            if (ucd_get_lexical_class(_cp & 0x1f'ffff) == unicode_lexical_class::white_space) {
                capture(_cp);
                advance_counters();
                _cp = advance();
                return token::none;

            } else {
                _state = state_type::idle;
                return token::ws;
            }
        }

        [[nodiscard]] constexpr token::kind_type parse_token_unicode_idle() noexcept
        {
            switch (ucd_get_lexical_class(_cp & 0x1f'ffff)) {
            case unicode_lexical_class::id_start:
                _state = state_type::identifier;
                capture(_cp);
                advance_counters();
                _cp = advance();
                return token::none;

            case unicode_lexical_class::white_space:
                _state = state_type::white_space;
                capture(_cp);
                advance_counters();
                _cp = advance();
                return token::none;

            case unicode_lexical_class::syntax:
                _state = state_type::idle;
                capture(_cp);
                advance_counters();
                _cp = advance();
                return token::other;

            default:
                capture(_cp);
                advance_counters();
                _cp = advance();
                return token::error_unexepected_character;
            }
        }

        [[nodiscard]] hi_no_inline constexpr token::kind_type parse_token_unicode() noexcept
        {
            using enum state_type;

            // Unicode by-pass.
            switch (_state) {
            case idle:
                return parse_token_unicode_idle();

            case white_space:
                return parse_token_unicode_white_space();

            case line_comment:
                return parse_token_unicode_line_comment();

            case identifier:
                return parse_token_unicode_identifier();

            case dqstring_literal:
            case sqstring_literal:
            case bqstring_literal:
            case block_comment:
                capture(_cp);
                advance_counters();
                _cp = advance();
                return token::none;

            case ini_string:
                // Line-feeds will terminate an ini-string.
                if (_cp == U'\u0085' or _cp == U'\u2028' or _cp == U'\u2029') {
                    return token::istr;
                } else {
                    capture(_cp);
                    advance_counters();
                    _cp = advance();
                    return token::none;
                }

            default:
                // Most tokens are terminated when a non-ascii code-point is found.
                // Terminate these tokens as if we reached end-of-file.
                while (_state != idle) {
                    if (auto token_kind = process_command(); token_kind != token::none) {
                        return token_kind;
                    }
                }
                return token::none;
            }
        }

        [[nodiscard]] constexpr token::kind_type process_command(char c = '\0') noexcept
        {
            hilet command = _lexer->get_command(_state, c);
            _state = command.next_state;

            if (command.clear) {
                clear();
            }

            if (command.char_to_capture != '\0') {
                capture(command.char_to_capture);
            }

            if (command.advance) {
                if (command.advance_line) {
                    ++_line_nr;
                    _column_nr = 0;
                } else if (command.advance_tab) {
                    _column_nr /= 8;
                    ++_column_nr;
                    _column_nr *= 8;
                } else {
                    ++_column_nr;
                }
                _cp = advance();
            }

            return command.emit_token;
        }

        [[nodiscard]] constexpr token::kind_type parse_token() noexcept
        {
            _token.line_nr = _line_nr;
            _token.column_nr = _column_nr;
            clear();

            while (_cp <= 0x7fff'ffff) {
                if (_cp <= 0x7f) {
                    if (auto token_kind = process_command(char_cast<char>(_cp)); token_kind != token::none) {
                        return token_kind;
                    }

                } else {
                    auto emit_token = parse_token_unicode();
                    if (emit_token != token::none) {
                        return emit_token;
                    }
                }
            }

            // Handle trailing state changes at end-of-file.
            while (_state != state_type::idle) {
                if (auto token_kind = process_command(); token_kind != token::none) {
                    return token_kind;
                }
            }

            // We have finished parsing and there was no token captured.
            // For example when the end of file only contains white-space.
            return token::none;
        }
    };

    template<typename It, std::sentinel_for<It> ItEnd>
    [[nodiscard]] constexpr iterator<It, ItEnd> parse(It first, ItEnd last) const noexcept
    {
        return iterator<It, ItEnd>{this, first, last};
    }

    [[nodiscard]] constexpr auto parse(std::string_view str) const noexcept
    {
        return parse(str.begin(), str.end());
    }

private:
    /** A array of commands, one for each state and character.
     * The array is in state-major order.
     */
    using transition_table_type = std::array<command_type, to_underlying(state_type::_size) * 128>;

    transition_table_type _transition_table;

    constexpr void add_string_literal(
        char c,
        token::kind_type string_token,
        state_type string_literal,
        state_type string_literal_quote,
        state_type string_literal_escape) noexcept
    {
        using enum state_type;

        add(idle, c, string_literal, advance);
        add(string_literal, any, idle, token::error_incomplete_string);
        for (uint8_t i = 1; i != 128; ++i) {
            if (char_cast<char>(i) != c and char_cast<char>(i) != '\\') {
                add(string_literal, char_cast<char>(i), string_literal, advance, capture);
            }
        }

        if constexpr (Config.escape_by_quote_doubling) {
            // Don't capture the first quote.
            add(string_literal, c, string_literal_quote, advance);
            // If quote is not doubled, this is the end of the string.
            add(string_literal_quote, any, idle, string_token);
            // Capture one quote of a doubled quote.
            add(string_literal_quote, c, string_literal, advance, capture);
        } else {
            // Quote ends the string.
            add(string_literal, c, idle, advance, string_token);
        }

        // Make sure that any escaped character sequence stays inside the string literal.
        add(string_literal, '\\', string_literal_escape, advance, capture);
        add(string_literal_escape, any, idle, token::error_incomplete_string);
        for (uint8_t i = 1; i != 128; ++i) {
            add(string_literal_escape, char_cast<char>(i), string_literal, advance, capture);
        }
    }

    constexpr void add_string_literals() noexcept
    {
        using enum state_type;

        if constexpr (Config.has_single_quote_string_literal) {
            add_string_literal('\'', token::sstr, sqstring_literal, sqstring_literal_quote, sqstring_literal_escape);
        } else {
            add(idle, '\'', idle, token::other, advance, capture);
        }

        if constexpr (Config.has_double_quote_string_literal) {
            add_string_literal('"', token::dstr, dqstring_literal, dqstring_literal_quote, dqstring_literal_escape);
        } else {
            add(idle, '"', idle, token::other, advance, capture);
        }

        if constexpr (Config.has_back_quote_string_literal) {
            add_string_literal('`', token::bstr, bqstring_literal, bqstring_literal_quote, bqstring_literal_escape);
        } else {
            add(idle, '`', idle, token::other, advance, capture);
        }
    }

    constexpr void add_number_literals() noexcept
    {
        using enum state_type;

        add(idle, "0", zero, advance, capture);
        add(idle, "123456789", dec_integer, advance, capture);

        add(zero, any, idle, token::integer);
        add(zero, "bB", bin_integer, advance, capture);
        add(zero, "oO", oct_integer, advance, capture);
        add(zero, "dD", dec_integer, advance, capture);
        add(zero, "xX", hex_integer, advance, capture);

        if constexpr (Config.zero_starts_octal) {
            add(zero, "01234567", oct_integer, advance, capture);
            add(zero, "89", idle, token::error_invalid_digit);
        } else {
            add(zero, "0123456789", dec_integer, advance, capture);
        }

        // binary-integer
        add(bin_integer, any, idle, token::integer);
        add(bin_integer, "01", bin_integer, advance, capture);
        add(bin_integer, "23456789", idle, token::error_invalid_digit);

        // octal-integer
        add(oct_integer, any, idle, token::integer);
        add(oct_integer, "01234567", oct_integer, advance, capture);
        add(oct_integer, "89", idle, token::error_invalid_digit);

        // decimal-integer
        add(dec_integer, any, idle, token::integer);
        add(dec_integer, "0123456789", dec_integer, advance, capture);
        add(dec_integer, ".", dec_float, advance, capture);
        add(dec_integer, "eE", dec_sign_exponent, advance, capture);

        // hexadecimal-integer
        add(hex_integer, any, idle, token::integer);
        add(hex_integer, "0123456789abcdefABCDEF", hex_integer, advance, capture);
        add(hex_integer, ".", hex_float, advance, capture);
        add(hex_integer, "pP", hex_sign_exponent, advance, capture);

        // decimal-float
        add(found_dot, "0123456789eE", dec_float);
        add(dec_float, any, idle, token::real);
        add(dec_float, "0123456789", dec_float, advance, capture);
        add(dec_float, "eE", dec_sign_exponent, advance, capture);
        add(dec_sign_exponent, any, idle, token::error_incomplete_exponent);
        add(dec_sign_exponent, "0123456789", dec_exponent_more, advance, capture);
        add(dec_sign_exponent, "+-", dec_exponent, advance, capture);
        add(dec_exponent, any, idle, token::error_incomplete_exponent);
        add(dec_exponent, "0123456789", dec_exponent_more, advance, capture);
        add(dec_exponent_more, any, idle, token::real);
        add(dec_exponent_more, "0123456789", dec_exponent_more, advance, capture);

        // hexadecimal-float
        add(hex_float, any, idle, token::real);
        add(hex_float, "0123456789abcdefABCDEF", hex_float, advance, capture);
        add(hex_float, "pP", hex_sign_exponent, advance, capture);
        add(hex_sign_exponent, any, idle, token::error_incomplete_exponent);
        add(hex_sign_exponent, "0123456789abcdefABCDEF", hex_exponent_more, advance, capture);
        add(hex_sign_exponent, "+-", hex_exponent, advance, capture);
        add(hex_exponent, any, idle, token::error_incomplete_exponent);
        add(hex_exponent, "0123456789abcdefABCDEF", hex_exponent_more, advance, capture);
        add(hex_exponent_more, any, idle, token::real);
        add(hex_exponent_more, "0123456789abcdefABCDEF", hex_exponent_more, advance, capture);

        if constexpr (Config.digit_separator != '\0') {
            if constexpr (Config.zero_starts_octal) {
                add(zero, Config.digit_separator, oct_integer, advance);
            } else {
                add(zero, Config.digit_separator, dec_integer, advance);
            }
            add(bin_integer, Config.digit_separator, bin_integer, advance);
            add(oct_integer, Config.digit_separator, oct_integer, advance);
            add(dec_integer, Config.digit_separator, dec_integer, advance);
            add(hex_integer, Config.digit_separator, hex_integer, advance);
            add(dec_float, Config.digit_separator, dec_integer, advance);
            add(hex_float, Config.digit_separator, dec_integer, advance);
            add(dec_exponent, Config.digit_separator, dec_integer, advance);
            add(hex_exponent, Config.digit_separator, dec_integer, advance);
        }
    }

    constexpr void add_color_literal() noexcept
    {
        using enum state_type;

        if constexpr (Config.has_color_literal) {
            add(found_hash, "0123456789abcdefABCDEF", color_literal, clear, capture, advance);
            add(color_literal, any, idle, token::color);
            add(color_literal, "0123456789abcdefABCDEF", color_literal, advance, capture);
        }
    }

    constexpr void add_ini_assignment() noexcept
    {
        using enum state_type;

        if constexpr (Config.equal_is_ini_assignment) {
            // Ignore white-space
            add(found_eq, " \t", found_eq, advance);
            add(found_eq, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_", ini_string, token::other);
        }

        if constexpr (Config.colon_is_ini_assignment) {
            // Ignore white-space
            add(found_colon, " \t", found_colon, advance);
            add(found_colon, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_", ini_string, token::other);
        }

        add(ini_string, any, idle, token::istr);
        add(ini_string, excluding("\n\v\f\r\0"), ini_string, advance, capture);
        add(ini_string, '\r', ini_string, advance);
    }

    constexpr void add_comments() noexcept
    {
        using enum state_type;

        if constexpr (Config.has_double_slash_line_comment) {
            add(found_slash, '/', line_comment, clear, advance);
        }

        if constexpr (Config.has_semicolon_line_comment) {
            add(idle, ';', line_comment, advance);
        } else {
            add(idle, ';', idle, token::other, capture, advance);
        }

        if constexpr (Config.has_hash_line_comment) {
            add(found_hash, excluding("\0"), line_comment, clear, advance, capture);
        }

        if constexpr (Config.has_c_block_comment) {
            add(found_slash, '*', block_comment, advance, clear);
        }

        if constexpr (Config.has_sgml_block_comment) {
            add(found_lt, '!', found_lt_bang, advance);
            add(found_lt_bang, any, idle, token::error_after_lt_bang);
            add(found_lt_bang, '-', found_lt_bang_dash, advance);
            add(found_lt_bang_dash, any, idle, token::error_after_lt_bang);
            add(found_lt_bang_dash, '-', block_comment, advance);
        }

        add(line_comment, any, idle, token::lcomment);
        add(line_comment, excluding("\r\n\f\v\0"), line_comment, advance, capture);

        add(line_comment, '\r', line_comment, advance);
        add(line_comment, "\n\f\v", idle, advance, token::lcomment);

        add(block_comment, any, idle, token::error_incomplete_comment);

        static_assert(Config.has_c_block_comment == 0 or Config.has_sgml_block_comment == 0);

        if constexpr (Config.has_c_block_comment) {
            add(block_comment, excluding("*\0"), block_comment, advance, capture);
            add(block_comment, '*', block_comment_found_star, advance);
            add(block_comment_found_star, any, block_comment, '*');
            add(block_comment_found_star, '/', idle, advance, token::bcomment);

        } else if constexpr (Config.has_sgml_block_comment) {
            add(block_comment, excluding("-\0"), block_comment, advance, capture);
            add(block_comment, '-', block_comment_found_dash, advance);
            add(block_comment_found_dash, any, block_comment, '-');
            add(block_comment_found_dash, '-', block_comment_found_dash_dash, advance);
            add(block_comment_found_dash_dash, any, block_comment_found_dash_dash_fin0, '-');
            add(block_comment_found_dash_dash_fin0, any, block_comment, '-');
            add(block_comment_found_dash_dash, '>', idle, advance, token::bcomment);
        }
    }

    constexpr void add_white_space() noexcept
    {
        using enum state_type;

        add(idle, '\r', white_space, advance);
        add(idle, " \n\t\v\f", white_space, advance, capture);
        add(white_space, any, idle, token::ws);
        add(white_space, '\r', white_space, advance);
        add(white_space, " \n\t\v\f", white_space, advance, capture);
    }

    constexpr void add_identifier() noexcept
    {
        using enum state_type;

        add(idle, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_", identifier, advance, capture);
        add(identifier, any, idle, token::id);
        add(identifier, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_0123456789", identifier, advance, capture);
    }

    constexpr command_type& _add(state_type from, char c, state_type to) noexcept
    {
        auto& command = get_command(from, c);
        command.next_state = to;
        command.char_to_capture = '\0';
        command.advance = 0;
        command.advance_line = 0;
        command.advance_tab = 0;
        command.clear = 0;
        command.emit_token = token::none;
        return command;
    }

    /** Add a state change.
     *
     * The attribute-arguments may be:
     * - token: The token will be emitted and the capture-buffer is reset.
     * - no_read_tag: The current character will not advance.
     * - reset_tag: Reset the capture-buffer explicitly.
     * - char: The character to capture, the default is @a c, if no_capture no character is captured.
     *
     * @param from The current state.
     * @param c The current character.
     * @param to The next state.
     * @param first The first attribute-argument
     * @param args The rest of the attribute-arguments.
     */
    template<typename First, typename... Args>
    constexpr command_type& _add(state_type from, char c, state_type to, First first, Args const&...args) noexcept
    {
        auto& command = _add(from, c, to, args...);
        if constexpr (std::is_same_v<First, token::kind_type>) {
            command.emit_token = first;

        } else if constexpr (std::is_same_v<First, advance_tag>) {
            command.advance = 1;
            if (c == '\n' or c == '\v' or c == '\f') {
                command.advance_line = 1;
            } else if (c == '\t') {
                command.advance_tab = 1;
            }

        } else if constexpr (std::is_same_v<First, clear_tag>) {
            command.clear = 1;

        } else if constexpr (std::is_same_v<First, capture_tag>) {
            command.char_to_capture = c;

        } else if constexpr (std::is_same_v<First, char>) {
            command.char_to_capture = first;

        } else {
            hi_static_no_default();
        }

        return command;
    }

    template<typename... Args>
    constexpr void add(state_type from, char c, state_type to, Args const&...args) noexcept
    {
        auto& command = _add(from, c, to, args...);
        hi_assert(not command.assigned, "Overwriting an already assigned state:char combination.");
        command.assigned = true;
    }

    template<typename... Args>
    constexpr void add(state_type from, std::string_view str, state_type to, Args const&...args) noexcept
    {
        for (auto c : str) {
            auto& command = _add(from, c, to, args...);
            hi_assert(not command.assigned, "Overwriting an already assigned state:char combination.");
            command.assigned = true;
        }
    }

    template<typename... Args>
    constexpr void add(state_type from, any_tag, state_type to, Args const&...args) noexcept
    {
        static_assert(not has_advance_tag_argument<Args...>(), "any should not advance");

        for (uint8_t c = 0; c != 128; ++c) {
            hilet& command = _add(from, char_cast<char>(c), to, args...);
            hi_assert(not command.assigned, "any should be added first to a state");
        }
    }

    template<typename... Args>
    constexpr void add(state_type from, excluding_tag const& exclusions, state_type to, Args const&...args) noexcept
    {
        for (uint8_t c = 0; c != 128; ++c) {
            if (not exclusions.contains(char_cast<char>(c))) {
                auto& command = _add(from, char_cast<char>(c), to, args...);
                hi_assert(not command.assigned, "Overwriting an already assigned state:char combination.");
                command.assigned = true;
            }
        }
    }
};

} // namespace detail

template<lexer_config Config>
constexpr auto lexer = detail::lexer<Config>();

}} // namespace hi::v1