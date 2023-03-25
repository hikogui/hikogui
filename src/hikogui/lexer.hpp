
#pragma once

#include "utility/module.hpp"
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
    uint16_t whitespace_is_token : 1 = 0;

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
        r.zero_starts_octal = 1;
        r.digit_separator = '\'';
        r.has_double_quote_string_literal = 1;
        r.has_single_quote_string_literal = 1;
        r.has_double_slash_line_comment = 1;
        r.has_c_block_comment = 1;
        return r;
    }
};

enum class lexer_token_kind : uint8_t {
    none,
    error_missing_exponent_number,
    error_after_lt_bang,
    integer_literal,
    float_literal,
    sqstring_literal,
    dqstring_literal,
    bqstring_literal,
    line_comment,
    block_comment,
    separator,
    whitespace,
    identifier,
    _operator
};

// clang-format off
constexpr auto lexer_token_kind_metadata = enum_metadata{
    lexer_token_kind::none, "none",
    lexer_token_kind::error_missing_exponent_number, "error:missing_exponent_number",
    lexer_token_kind::error_after_lt_bang, "error:after_lt_bang",
    lexer_token_kind::integer_literal, "integer",
    lexer_token_kind::float_literal, "float",
    lexer_token_kind::sqstring_literal, "single-quote string",
    lexer_token_kind::dqstring_literal, "double-quote string",
    lexer_token_kind::bqstring_literal, "back-quote string",
    lexer_token_kind::line_comment, "line comment",
    lexer_token_kind::block_comment, "block comment",
    lexer_token_kind::separator, "separator",
    lexer_token_kind::whitespace, "whitespace",
    lexer_token_kind::identifier, "identifier",
    lexer_token_kind::_operator, "operator"
};
// clang-format on

struct lexer_token_type {
    lexer_token_kind kind;
    std::string capture;
    size_t offset;

    [[nodiscard]] constexpr friend bool operator==(lexer_token_type const&, lexer_token_type const&) noexcept = default;

    inline friend std::ostream& operator<<(std::ostream& lhs, hi::lexer_token_type const& rhs)
    {
        return lhs << std::format("{}", rhs);
    }
};

namespace detail {

enum class lexer_state_type : uint8_t {
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
    sqstring_literal,
    sqstring_literal_quote,
    sqstring_literal_escape,
    dqstring_literal,
    dqstring_literal_quote,
    dqstring_literal_escape,
    bqstring_literal,
    bqstring_literal_quote,
    bqstring_literal_escape,
    comment_found_slash,
    comment_found_lt,
    comment_found_lt_bang,
    comment_found_lt_bang_dash,
    block_comment_found_star,
    block_comment_found_dash,
    block_comment_found_dash_dash,
    block_comment_found_dash_dash_fin0,
    line_comment,
    block_comment,
    found_slash,
    found_lt,
    found_lt_eq,
    found_semicolon,
    found_hash,
    found_eq,
    found_eq_eq,
    whitespace,
    _operator,
    identifier,

    _size
};

struct lexer_clear_tag {};
struct lexer_any_tag {};
struct lexer_advance_tag {};
struct lexer_capture_tag {};

/** Capture a character in the lexer_capture buffer.
 */
constexpr auto lexer_capture = lexer_capture_tag{};

/** Advance the input iterator.
 */
constexpr auto lexer_advance = lexer_advance_tag{};

/** Clear the capture buffer.
 */
constexpr auto lexer_clear = lexer_clear_tag{};

/** Match any character.
 */
constexpr auto lexer_any = lexer_any_tag{};

/** This is the command to execute for a given state and given character.
 */
struct lexer_command_type {
    /** The state to switch to.
     */
    lexer_state_type next_state = lexer_state_type::idle;

    /** The token to emit.
     */
    lexer_token_kind emit_token = lexer_token_kind::none;

    /** The char to lexer_capture.
     */
    char char_to_capture = '\0';

    /** Clear the lexer_capture buffer.
     */
    uint8_t clear : 1 = 0;

    /** Advance the iterator.
     */
    uint8_t advance : 1 = 0;

    /** This entry has been assigned.
     */
    uint8_t assigned : 1 = 0;
};

template<lexer_config Config>
class lexer {
public:
    constexpr lexer() noexcept : _transition_table()
    {
        add_literal_numbers();
        if constexpr (Config.has_single_quote_string_literal) {
            add_literal_string(
                '\'',
                lexer_token_kind::sqstring_literal,
                lexer_state_type::sqstring_literal,
                lexer_state_type::sqstring_literal_quote,
                lexer_state_type::sqstring_literal_escape);
        }
        if constexpr (Config.has_double_quote_string_literal) {
            add_literal_string(
                '"',
                lexer_token_kind::dqstring_literal,
                lexer_state_type::dqstring_literal,
                lexer_state_type::dqstring_literal_quote,
                lexer_state_type::dqstring_literal_escape);
        }
        if constexpr (Config.has_back_quote_string_literal) {
            add_literal_string(
                '`',
                lexer_token_kind::btstring_literal,
                lexer_state_type::btstring_literal,
                lexer_state_type::btstring_literal_quote,
                lexer_state_type::btstring_literal_escape);
        }
        if constexpr (Config.has_color_literal) {
            add_literal_color();
        }
        add_comments();
        add_operators();
        add_separators();
        add_whitespace();
        add_identifier();
    }

    [[nodiscard]] constexpr lexer_command_type& get_command(lexer_state_type from, char c) noexcept
    {
        return _transition_table[to_underlying(from) * 128_uz + char_cast<size_t>(c)];
    }

    [[nodiscard]] constexpr lexer_command_type const& get_command(lexer_state_type from, char c) const noexcept
    {
        return _transition_table[to_underlying(from) * 128_uz + char_cast<size_t>(c)];
    }

    template<typename It, std::sentinel_for<It> ItEnd>
    struct iterator {
    public:
        constexpr iterator(lexer const *lexer, It first, ItEnd last) noexcept : _lexer(lexer), _it(first), _last(last)
        {
            parse_token();
        }

        [[nodiscard]] constexpr lexer_token_type const& operator*() const noexcept
        {
            return _token;
        }

        constexpr iterator& operator++() noexcept
        {
            hi_axiom(not _finished);
            parse_token();
            return *this;
        }

        [[nodiscard]] constexpr bool operator==(std::default_sentinel_t) const noexcept
        {
            return _finished;
        }

    private:
        lexer const *_lexer;
        It _it;
        ItEnd _last;
        size_t _offset = 0;
        lexer_token_type _token;
        lexer_state_type _current_state = lexer_state_type::idle;
        bool _finished = false;

        constexpr void parse_token() noexcept
        {
            _token.kind = lexer_token_kind::none;
            _token.capture.clear();
            _token.offset = _offset;

            if (_it == _last) {
                _finished = true;
                return;
            }

            while (_it != _last) {
                hilet c = *_it;
                hilet& command = _lexer->get_command(_current_state, c);
                _current_state = command.next_state;

                if (command.clear) {
                    _token.capture.clear();
                }

                if (command.char_to_capture != '\0') {
                    _token.capture += command.char_to_capture;
                }

                if (command.advance) {
                    hi_axiom(_it != _last);
                    ++_it;
                    ++_offset;
                }

                if (command.emit_token != lexer_token_kind::none) {
                    _token.kind = command.emit_token;
                    return;
                }
            }

            // Handle trailing state changes at end-of-file.
            while (_current_state != lexer_state_type::idle) {
                hilet& command = _lexer->get_command(_current_state, '\0');
                _current_state = command.next_state;

                if (command.clear) {
                    _token.capture.clear();
                }

                if (command.char_to_capture != '\0') {
                    _token.capture += command.char_to_capture;
                }

                hi_axiom(not command.advance);

                if (command.emit_token != lexer_token_kind::none) {
                    _token.kind = command.emit_token;
                    return;
                }
            }
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
    using transition_table_type = std::array<lexer_command_type, to_underlying(lexer_state_type::_size) * 128>;

    transition_table_type _transition_table;

    constexpr void add_literal_numbers() noexcept
    {
        add(lexer_state_type::idle, "0", lexer_state_type::zero, lexer_advance, lexer_capture);
        add(lexer_state_type::idle, "123456789", lexer_state_type::dec_integer, lexer_advance, lexer_capture);

        add(lexer_state_type::zero, lexer_any, lexer_state_type::idle, lexer_token_kind::integer_literal);
        add(lexer_state_type::zero, "bB", lexer_state_type::bin_integer, lexer_advance, lexer_capture);
        add(lexer_state_type::zero, "oO", lexer_state_type::oct_integer, lexer_advance, lexer_capture);
        add(lexer_state_type::zero, "dD", lexer_state_type::dec_integer, lexer_advance, lexer_capture);
        add(lexer_state_type::zero, "xX", lexer_state_type::hex_integer, lexer_advance, lexer_capture);

        if constexpr (Config.zero_starts_octal) {
            add(lexer_state_type::zero, "01234567", lexer_state_type::oct_integer, lexer_advance, lexer_capture);
        } else {
            add(lexer_state_type::zero, "0123456789", lexer_state_type::dec_integer, lexer_advance, lexer_capture);
        }

        if constexpr (Config.digit_separator != '\0') {
            if constexpr (Config.zero_starts_octal) {
                add(lexer_state_type::zero, Config.digit_separator, lexer_state_type::oct_integer, lexer_advance);
            } else {
                add(lexer_state_type::zero, Config.digit_separator, lexer_state_type::dec_integer, lexer_advance);
            }
            add(lexer_state_type::bin_integer, Config.digit_separator, lexer_state_type::bin_integer, lexer_advance);
            add(lexer_state_type::oct_integer, Config.digit_separator, lexer_state_type::oct_integer, lexer_advance);
            add(lexer_state_type::dec_integer, Config.digit_separator, lexer_state_type::dec_integer, lexer_advance);
            add(lexer_state_type::hex_integer, Config.digit_separator, lexer_state_type::hex_integer, lexer_advance);
            add(lexer_state_type::dec_float, Config.digit_separator, lexer_state_type::dec_integer, lexer_advance);
            add(lexer_state_type::hex_float, Config.digit_separator, lexer_state_type::dec_integer, lexer_advance);
            add(lexer_state_type::dec_exponent, Config.digit_separator, lexer_state_type::dec_integer, lexer_advance);
            add(lexer_state_type::hex_exponent, Config.digit_separator, lexer_state_type::dec_integer, lexer_advance);
        }

        // binary-integer
        add(lexer_state_type::bin_integer, lexer_any, lexer_state_type::idle, lexer_token_kind::integer_literal);
        add(lexer_state_type::bin_integer, "01", lexer_state_type::bin_integer, lexer_advance, lexer_capture);

        // octal-integer
        add(lexer_state_type::oct_integer, lexer_any, lexer_state_type::idle, lexer_token_kind::integer_literal);
        add(lexer_state_type::oct_integer, "01234567", lexer_state_type::oct_integer, lexer_advance, lexer_capture);

        // decimal-integer
        add(lexer_state_type::dec_integer, lexer_any, lexer_state_type::idle, lexer_token_kind::integer_literal);
        add(lexer_state_type::dec_integer, "0123456789", lexer_state_type::dec_integer, lexer_advance, lexer_capture);
        add(lexer_state_type::dec_integer, ".", lexer_state_type::dec_float, lexer_advance, lexer_capture);
        add(lexer_state_type::dec_integer, "eE", lexer_state_type::dec_sign_exponent, lexer_advance, lexer_capture);

        // hexadecimal-integer
        add(lexer_state_type::hex_integer, lexer_any, lexer_state_type::idle, lexer_token_kind::integer_literal);
        add(lexer_state_type::hex_integer, "0123456789abcdefABCDEF", lexer_state_type::hex_integer, lexer_advance, lexer_capture);
        add(lexer_state_type::hex_integer, ".", lexer_state_type::hex_float, lexer_advance, lexer_capture);
        add(lexer_state_type::hex_integer, "pP", lexer_state_type::hex_sign_exponent, lexer_advance, lexer_capture);

        // decimal-float
        add(lexer_state_type::dec_float, lexer_any, lexer_state_type::idle, lexer_token_kind::float_literal);
        add(lexer_state_type::dec_float, "0123456789", lexer_state_type::dec_float, lexer_advance, lexer_capture);
        add(lexer_state_type::dec_float, "eE", lexer_state_type::dec_sign_exponent, lexer_advance, lexer_capture);
        add(lexer_state_type::dec_sign_exponent,
            lexer_any,
            lexer_state_type::idle,
            lexer_advance,
            lexer_capture,
            lexer_token_kind::error_missing_exponent_number);
        add(lexer_state_type::dec_sign_exponent, "0123456789", lexer_state_type::dec_exponent, lexer_advance, lexer_capture);
        add(lexer_state_type::dec_sign_exponent, "+-", lexer_state_type::dec_exponent, lexer_advance, lexer_capture);
        add(lexer_state_type::dec_exponent, lexer_any, lexer_state_type::idle, lexer_token_kind::float_literal);
        add(lexer_state_type::dec_exponent, "0123456789", lexer_state_type::dec_exponent, lexer_advance, lexer_capture);

        // hexadecimal-float
        add(lexer_state_type::hex_float, lexer_any, lexer_state_type::idle, lexer_token_kind::float_literal);
        add(lexer_state_type::hex_float, "0123456789abcdefABCDEF", lexer_state_type::hex_float, lexer_advance, lexer_capture);
        add(lexer_state_type::hex_float, "pP", lexer_state_type::hex_sign_exponent, lexer_advance, lexer_capture);
        add(lexer_state_type::hex_sign_exponent,
            lexer_any,
            lexer_state_type::idle,
            lexer_advance,
            lexer_capture,
            lexer_token_kind::error_missing_exponent_number);
        add(lexer_state_type::hex_sign_exponent,
            "0123456789abcdefABCDEF",
            lexer_state_type::hex_exponent,
            lexer_advance,
            lexer_capture);
        add(lexer_state_type::hex_sign_exponent, "+-", lexer_state_type::hex_exponent, lexer_advance, lexer_capture);
        add(lexer_state_type::hex_exponent,
            lexer_any,
            lexer_state_type::idle,
            lexer_advance,
            lexer_capture,
            lexer_token_kind::float_literal);
        add(lexer_state_type::hex_exponent,
            "0123456789abcdefABCDEF",
            lexer_state_type::hex_exponent,
            lexer_advance,
            lexer_capture);
    }

    constexpr void add_literal_string(
        char c,
        lexer_token_kind string_token,
        lexer_state_type string_literal,
        lexer_state_type string_literal_quote,
        lexer_state_type string_literal_escape) noexcept
    {
        add(lexer_state_type::idle, c, string_literal, lexer_advance);
        add(string_literal, lexer_any, string_literal, lexer_advance, lexer_capture);

        if constexpr (Config.escape_by_quote_doubling) {
            // Don't catpure the first quote.
            add(string_literal, c, string_literal_quote, lexer_advance);
            // If quote is not doubled, this is the end of the string.
            add(string_literal_quote, lexer_any, lexer_state_type::idle, string_token);
            // Capture one quote of a doubled quote.
            add(string_literal_quote, c, string_literal, lexer_advance, lexer_capture);

        } else {
            // Quote ends the string.
            add(string_literal, c, lexer_state_type::idle, lexer_advance, string_token);

            // Make sure that lexer_any escaped character sequence stays inside the string literal.
            add(string_literal, "\\", string_literal_escape, lexer_advance, lexer_capture);
            add(string_literal_escape, lexer_any, lexer_state_type::idle, string_token, lexer_advance, lexer_capture);
        }
    }

    constexpr void add_literal_color() noexcept
    {
        add(lexer_state_type::idle, "#+=", lexer_state_type::literal_color, lexer_advance, lexer_capture);
        add(lexer_state_type::literal_color, lexer_any, lexer_state_type::idle, lexer_token_kind::color_literal);
        add(lexer_state_type::literal_color,
            "0123456789abcdefABCDEF+=",
            lexer_state_type::literal_color,
            lexer_advance,
            lexer_capture);
    }

    constexpr void add_operators() noexcept
    {
        // When the operator starts with an equal sign, then it may be one of the following operators:
        //  - "=": assignment
        //  - "==": equal
        //  - "===": identical
        add(lexer_state_type::idle, "=", lexer_state_type::found_eq, lexer_advance, lexer_capture);
        add(lexer_state_type::found_eq, lexer_any, lexer_state_type::idle, lexer_token_kind::_operator);
        add(lexer_state_type::found_eq, "=", lexer_state_type::found_eq_eq, lexer_advance, lexer_capture);
        add(lexer_state_type::found_eq_eq, lexer_any, lexer_state_type::idle, lexer_token_kind::_operator);
        add(lexer_state_type::found_eq_eq,
            "=",
            lexer_state_type::idle,
            lexer_token_kind::_operator,
            lexer_advance,
            lexer_capture);

        // Any other mathematical symbol continues a multi-character operator.
        add(lexer_state_type::idle, ":.+-*/%~&|^!<>?@$", lexer_state_type::_operator, lexer_advance, lexer_capture);

        // add_comments() will add the singular slash
        // Treat it as if a mathematical symbol was read and continue a multi-character operator.
        add(lexer_state_type::found_slash, lexer_any, lexer_state_type::_operator);

        // Read the multi-character operator until a non mathematical symbol.
        add(lexer_state_type::_operator, lexer_any, lexer_state_type::idle, lexer_token_kind::_operator);
        add(lexer_state_type::_operator, ":.,+-*/%~&|^!<>?@$", lexer_state_type::_operator, lexer_advance, lexer_capture);

        // An equal sign ends a multi-character operator as a inplace-assignment or comparison.
        add(lexer_state_type::_operator, "=", lexer_state_type::idle, lexer_token_kind::_operator, lexer_advance, lexer_capture);

        // Add_comments() will add the singular less-than.
        // There are few special operators that start with less than
        //  - "<=" less than or equal
        //  - "<=>" spaceship operator
        add(lexer_state_type::found_lt, lexer_any, lexer_state_type::idle, lexer_token_kind::_operator);
        add(lexer_state_type::found_lt, ":.,+-*/%~&|^!<>?@$", lexer_state_type::_operator, lexer_advance, lexer_capture);
        add(lexer_state_type::found_lt, "=", lexer_state_type::found_lt_eq, lexer_advance, lexer_capture);
        add(lexer_state_type::found_lt_eq, lexer_any, lexer_state_type::idle, lexer_token_kind::_operator);
        add(lexer_state_type::found_lt_eq,
            ">",
            lexer_state_type::idle,
            lexer_token_kind::_operator,
            lexer_advance,
            lexer_capture);
    }

    constexpr void add_comments() noexcept
    {
        if constexpr (Config.has_double_slash_line_comment or Config.has_c_block_comment) {
            // Add an indirection when starting comment with '/'.
            add(lexer_state_type::idle, '/', lexer_state_type::comment_found_slash, lexer_advance);
            add(lexer_state_type::comment_found_slash, lexer_any, lexer_state_type::found_slash, '/');
        } else {
            add(lexer_state_type::idle, '/', lexer_state_type::found_slash, lexer_advance, lexer_capture);
        }

        if constexpr (Config.has_sgml_block_comment) {
            // Add an indirection when starting comment with '<'.
            add(lexer_state_type::idle, "<", lexer_state_type::comment_found_lt, lexer_advance);
            add(lexer_state_type::comment_found_lt, lexer_any, lexer_state_type::found_lt, '<');
        } else {
            add(lexer_state_type::idle, '<', lexer_state_type::found_lt, lexer_advance, lexer_capture);
        }

        if constexpr (Config.has_double_slash_line_comment) {
            add(lexer_state_type::comment_found_slash, '/', lexer_state_type::line_comment);
        }

        if constexpr (Config.has_semicolon_line_comment) {
            add(lexer_state_type::idle, ';', lexer_state_type::line_comment, lexer_advance);
        } else {
            add(lexer_state_type::idle, ';', lexer_state_type::found_semicolon, lexer_advance, lexer_capture);
        }

        if constexpr (Config.has_hash_line_comment) {
            add(lexer_state_type::idle, '#', lexer_state_type::line_comment, lexer_advance);
        } else {
            add(lexer_state_type::idle, '#', lexer_state_type::found_hash, lexer_advance, lexer_capture);
        }

        if constexpr (Config.has_c_block_comment) {
            add(lexer_state_type::comment_found_slash, '*', lexer_state_type::block_comment, lexer_advance);
        }

        if constexpr (Config.has_sgml_block_comment) {
            add(lexer_state_type::comment_found_lt, '!', lexer_state_type::comment_found_lt_bang, lexer_advance);
            add(lexer_state_type::comment_found_lt_bang,
                lexer_any,
                lexer_state_type::idle,
                lexer_token_kind::error_after_lt_bang);
            add(lexer_state_type::comment_found_lt_bang, '-', lexer_state_type::comment_found_lt_bang_dash, lexer_advance);
            add(lexer_state_type::comment_found_lt_bang_dash,
                lexer_any,
                lexer_state_type::idle,
                lexer_token_kind::error_after_lt_bang);
            add(lexer_state_type::comment_found_lt_bang_dash, '-', lexer_state_type::block_comment, lexer_advance);
        }

        add(lexer_state_type::line_comment, lexer_any, lexer_state_type::line_comment, lexer_advance, lexer_capture);
        add(lexer_state_type::line_comment, '\r', lexer_state_type::line_comment, lexer_advance);
        add(lexer_state_type::line_comment, "\n\f\v", lexer_state_type::idle, lexer_advance, lexer_token_kind::line_comment);

        add(lexer_state_type::block_comment, lexer_any, lexer_state_type::block_comment);

        if constexpr (Config.has_c_block_comment) {
            add(lexer_state_type::block_comment, '*', lexer_state_type::block_comment_found_star, lexer_advance);
            add(lexer_state_type::block_comment_found_star, lexer_any, lexer_state_type::block_comment, '*');
            add(lexer_state_type::block_comment_found_star,
                '/',
                lexer_state_type::idle,
                lexer_advance,
                lexer_token_kind::block_comment);
        }

        if constexpr (Config.has_sgml_block_comment) {
            add(lexer_state_type::block_comment, '-', lexer_state_type::block_comment_found_dash, lexer_advance);
            add(lexer_state_type::block_comment_found_dash, lexer_any, lexer_state_type::block_comment, '-');
            add(lexer_state_type::block_comment_found_dash, '-', lexer_state_type::block_comment_found_dash_dash, lexer_advance);
            add(lexer_state_type::block_comment_found_dash_dash,
                lexer_any,
                lexer_state_type::block_comment_found_dash_dash_fin0,
                '-');
            add(lexer_state_type::block_comment_found_dash_dash_fin0, lexer_any, lexer_state_type::block_comment, '-');
            add(lexer_state_type::block_comment_found_dash_dash,
                '>',
                lexer_state_type::idle,
                lexer_advance,
                lexer_token_kind::block_comment);
        }
    }

    constexpr void add_separators() noexcept
    {
        add(lexer_state_type::idle, ",()[]{}", lexer_state_type::idle, lexer_advance, lexer_capture, lexer_token_kind::separator);
        if constexpr (not Config.has_semicolon_line_comment) {
            // found semicolon is because of line comments.
            add(lexer_state_type::found_semicolon, lexer_any, lexer_state_type::idle, lexer_token_kind::separator);
        }
    }

    constexpr void add_whitespace() noexcept
    {
        if constexpr (Config.whitespace_is_token) {
            add(lexer_state_type::idle, '\r', lexer_state_type::idle, lexer_advance);
            add(lexer_state_type::idle, " \n\t\v\f", lexer_state_type::whitespace, lexer_advance, lexer_capture);
            add(lexer_state_type::whitespace, lexer_any, lexer_state_type::idle, lexer_token_kind::whitespace);
            add(lexer_state_type::whitespace, '\r', lexer_state_type::whitespace, lexer_advance);
            add(lexer_state_type::whitespace, " \n\t\v\f", lexer_state_type::whitesspace, lexer_advance, lexer_capture);
        } else {
            add(lexer_state_type::idle, " \r\n\t\v\f", lexer_state_type::idle, lexer_advance);
        }
    }

    constexpr void add_identifier() noexcept
    {
        add(lexer_state_type::idle,
            "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_",
            lexer_state_type::identifier,
            lexer_advance,
            lexer_capture);
        add(lexer_state_type::identifier, lexer_any, lexer_state_type::idle, lexer_token_kind::identifier);
        add(lexer_state_type::identifier,
            "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_0123456789",
            lexer_state_type::identifier,
            lexer_advance,
            lexer_capture);
    }

    constexpr lexer_command_type& add(lexer_state_type from, char c, lexer_state_type to) noexcept
    {
        auto& command = get_command(from, c);
        command.next_state = to;
        command.char_to_capture = '\0';
        command.advance = 0;
        command.clear = 0;
        command.emit_token = lexer_token_kind::none;
        command.assigned = 1;
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
    constexpr lexer_command_type&
    add(lexer_state_type from, char c, lexer_state_type to, First first, Args const&...args) noexcept
    {
        auto& command = add(from, c, to, args...);
        if constexpr (std::is_same_v<First, lexer_token_kind>) {
            command.emit_token = first;

        } else if constexpr (std::is_same_v<First, lexer_advance_tag>) {
            command.advance = 1;

        } else if constexpr (std::is_same_v<First, lexer_clear_tag>) {
            command.clear = 1;

        } else if constexpr (std::is_same_v<First, lexer_capture_tag>) {
            command.char_to_capture = c;

        } else if constexpr (std::is_same_v<First, char>) {
            command.char_to_capture = first;

        } else {
            hi_static_no_default();
        }

        return command;
    }

    template<typename... Args>
    constexpr void add(lexer_state_type from, std::string_view str, lexer_state_type to, Args const&...args) noexcept
    {
        for (auto c : str) {
            add(from, c, to, args...);
        }
    }

    template<typename... Args>
    constexpr void add(lexer_state_type from, lexer_any_tag, lexer_state_type to, Args const&...args) noexcept
    {
        for (uint8_t c = 0; c != 128; ++c) {
            add(from, char_cast<char>(c), to, args...);
        }
    }
};

} // namespace detail

template<lexer_config Config>
constexpr auto lexer = detail::lexer<Config>();

}} // namespace hi::v1

template<typename CharT>
struct std::formatter<hi::lexer_token_type, CharT> : std::formatter<std::string, CharT> {
    auto format(hi::lexer_token_type const& t, auto& fc)
    {
        return std::formatter<std::string, CharT>::format(
            std::format("{} \"{}\" {}", hi::lexer_token_kind_metadata[t.kind], t.capture, t.offset), fc);
    }
};
