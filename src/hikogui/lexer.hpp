
#pragma once

#include "utility/module.hpp"

namespace hi { inline namespace v1 {

struct lexer_config {
    /** A zero starts in octal number.
     *
     * By default a zero starts a decimal number, but some languages
     * like C and C++ start an octal number with zero.
     */
    bool zero_starts_octal = false;

    /** The character used to separate groups of numbers.
     *
     * This character is the character that will be ignored by a language
     * if it appears in a integer or floating point literal.
     *
     * For C and C++ this is the quote character, some other languages use
     * an underscore. If the language does not support group separator set this to '\0'.
     */
    char number_group_separator = '\0';

    /** Escaping quotes within a string may be done using quote doubling.
     */
    bool escape_by_quote_doubling = false;

    /** The language has a literal color.
     *
     * This is a hash '#' followed by a hexadecimal number.
     */
    bool has_color_literal = false;

    bool has_double_quote_string_literal = true;
    bool has_single_quote_string_literal = true;
    bool has_back_quote_string_literal = false;

    bool double_slash_line_comment = true;
    bool hash_line_comment = false;
    bool semicoloon_line_comment = false;

    bool c_block_comment = true;
    bool sgml_block_comment = false;
};

namespace detail {

template<lexer_config Config>
class lexer {
public:
    enun class token : uint8_t {
        none
    };

private:
    enum class state_type : uint8_t {
        idle,
        zero,
        bin_integer,
        oct_integer,
        dec_integer,
        hex_integer,

        _size
    };

    struct clear_tag {};
    struct any_tag {};
    struct no_read_tag {};
    struct no_capture_tag {};

    constexpr static char no_capture = no_capture_tag{};
    constexpr static auto no_read = no_read_tag{};
    constexpr static auto clear = clear_tag{};
    constexpr static auto any = any_tag{};

    constexpr static auto idle = state_type::idle;
    constexpr static auto zero = state_type::zero;

    /** This is the command to execute for a given state and given character.
     */
    struct command_type {
        /** The state to switch to.
         * If state is set to state_type::_size then the command is not filled in yet.
         */
        state_type next_state = state_type::_size;

        /** The token to emit.
         * If this is token::none then no token is emitted.
         */
        token emit_token = token::none;

        /** The char to capture.
         * If this is nul, then nothing is captured.
         */
        char char_to_capture = no_capture;

        /** Clear the capture buffer.
         */
        uint8_t clear : 1 = 0;

        /** Read a character, and advance the iterator.
         */
        uint8_t read : 1 = 0;
    };

    /** A array of commands, one for each state and character.
     * The array is in state-major order.
     */
    using transition_table_type = std::array<command_type, to_underlying(state_type::_size) * 128>;

    transition_table_type _transition_table;

public:
    constexpr lexer() noexcept : _transition_table()
    {
        add_literal_numbers();
        if constexpr (Config.has_single_quote_string_literal) {
            add_literal_string(
                '\'',
                token::sqstring_literal,
                state::sqstring_literal,
                state::sqstring_literal_quote,
                state::sqstring_literal_escape);
        }
        if constexpr (Config.has_double_quote_string_literal) {
            add_literal_string(
                '"',
                token::dqstring_literal,
                state::dqstring_literal,
                state::dqstring_literal_quote,
                state::dqstring_literal_escape);
        }
        if constexpr (Config.has_back_quote_string_literal) {
            add_literal_string(
                '`',
                token::btstring_literal,
                state::btstring_literal,
                state::btstring_literal_quote,
                state::btstring_literal_escape);
        }
        if constexpr (Config.has_literal_color) {
            add_literal_color();
        }
        add_comments();
        add_operators();
        add_separators();
        add_identifier();
    }

    constexpr void add_literal_numbers() noexcept
    {
        add(state::idle, "0+=", state::zero);
        add(state::idle, "123456789+=", state::dec_integer);

        add(state::zero, "", state::idle, token::integer_literal);
        add(state::zero, "bB+=", state::bin_integer);
        add(state::zero, "oO+=", state::oct_integer);
        add(state::zero, "dD+=", state::dec_integer);
        add(state::zero, "xX+=", state::hex_integer);

        if constexpr (Config.zero_starts_octal) {
            add(state::zero, "01234567+=", state::oct_integer);
        } else {
            add(state::zero, "0123456789+=", state::dec_integer);
        }

        if constexpr (Config.quote_separates_digits != '\0') {
            if constexpr (Config.zero_starts_octal) {
                add(state::zero, "'+", state::oct_integer);
            } else {
                add(state::zero, "'+", state::dec_integer);
            }
            add(state::bin_integer, "'+", state::bin_integer);
            add(state::oct_integer, "'+", state::oct_integer);
            add(state::dec_integer, "'+", state::dec_integer);
            add(state::hex_integer, "'+", state::hex_integer);
            add(state::dec_float, "'+", state::dec_integer);
            add(state::hex_float, "'+", state::dec_integer);
            add(state::dec_exponent, "'+", state::dec_integer);
            add(state::hex_exponent, "'+", state::dec_integer);
        }

        // binary-integer
        add(state::bin_integer, "", state::idle, token::integer_literal);
        add(state::bin_integer, "01+=", state::bin_integer);

        // octal-integer
        add(state::oct_integer, "", state::idle, token::integer_literal);
        add(state::oct_integer, "01234567+=", state::oct_integer);

        // decimal-integer
        add(state::dec_integer, "", state::idle, token::integer_literal);
        add(state::dec_integer, "0123456789+=", state::dec_integer);
        add(state::dec_integer, ".+=", state::dec_float);
        add(state::dec_integer, "eE+=", state::dec_sign_exponent);

        // hexadecimal-integer
        add(state::hex_integer, "", state::idle, token::integer_literal);
        add(state::hex_integer, "0123456789abcdefABCDEF+=", state::hex_integer);
        add(state::hex_integer, ".+=", state::hex_float);
        add(state::hex_integer, "pP+=", state::hex_sign_exponent);

        // decimal-float
        add(state::dec_float, "", state::idle, token::float_literal);
        add(state::dec_float, "0123456789+=", state::dec_float);
        add(state::dec_float, "eE+=", state::dec_sign_exponent);
        add(state::dec_sign_exponent, "", state::idle, token::error_missing_exponent_number);
        add(state::dec_sign_exponent, "0123456789+=", state::dec_exponent);
        add(state::dec_sign_exponent, "+-", "+=", state::dec_exponent);
        add(state::dec_exponent, "", state::idle, token::float_literal);
        add(state::dec_exponent, "0123456789+=", state::dec_exponent);

        // hexadecimal-float
        add(state::hex_float, "", state::idle, token::float_literal);
        add(state::hex_float, "0123456789abcdefABCDEF+=", state::hex_float);
        add(state::hex_float, "pP+=", state::hex_sign_exponent);
        add(state::hex_sign_exponent, "", state::idle, token::error_missing_exponent_number);
        add(state::hex_sign_exponent, "0123456789abcdefABCDEF+=", state::hex_exponent);
        add(state::hex_sign_exponent, "+-", "+=", state::hex_exponent);
        add(state::hex_exponent, "", state::idle, token::float_literal);
        add(state::hex_exponent, "0123456789abcdefABCDEF+=", state::hex_exponent);
    }

    constexpr void add_literal_string(
        char c,
        token string_token,
        state_type string_literal,
        state_type string_literal_quote,
        state_type string_literal_escape) noexcept
    {
        add(state::idle, c, "+", state::string_literal);
        add(state::string_literal, "+=", state::string_literal);

        if constexpr (Config.escape_by_quote_doubling) {
            // Don't catpure the first quote.
            add(state::string_literal, c, "+", state::string_literal_quote);
            // If quote is not doubled, this is the end of the string.
            add(state::string_literal_quote, "", state::idle, token::string_literal);
            // Capture one quote of a doubled quote.
            add(state::string_literal_quote, c, "+=", state::string_literal);

        } else {
            // Quote ends the string.
            add(state::string_literal, c, "+", state::idle, token::string_literal);

            // Make sure that any escaped character sequence stays inside the string literal.
            add(state::string_literal, "\\", "+=", state::string_literal_escape);
            add(state::string_literal_escape, "+=", state::idle, string_literal);
        }
    }

    constexpr void add_literal_color() noexcept
    {
        add(state::idle, "#+=", state::literal_color);
        add(state::literal_color, "", state::idle, token::color_literal);
        add(state::literal_color, "0123456789abcdefABCDEF+=", state::literal_color);
    }

    constexpr void add_operators() noexcept
    {
        // When the operator starts with an equal sign, then it may be one of the following operators:
        //  - "=": assignment
        //  - "==": equal
        //  - "===": identical
        add(state::idle, "=", "+=", state::found_eq);
        add(state::found_eq, "", state::idle, state::_operator);
        add(state::found_eq, "=", "+=", state::found_eq_eq);
        add(state::found_eq_eq, "", state::idle, state::_operator);
        add(state::found_eq_eq, "=", "+=", state::idle, state::_operator);

        // Any other mathematical symbol continues a multi-character operator.
        add(state::idle, ":.+-*/%~&|^!<>?@$", "+=", state::_operator);

        // add_comments() will add the singular slash
        // Treat it as if a mathematical symbol was read and continue a multi-character operator.
        add(state::found_slash, "", state::found_operator);

        // Read the multi-character operator until a non mathematical symbol.
        add(state::_operator, "", state::idle, token::_operator);
        add(state::_operator, ":.,+-*/%~&|^!<>?@$", "+=", state::_operator);

        // An equal sign ends a multi-character operator as a inplace-assignment or comparison.
        add(state::_operator, "=", "+=", state::idle, token::_operator);

        // Add_comments() will add the singular less-than.
        // There are few special operators that start with less than
        //  - "<=" less than or equal
        //  - "<=>" spaceship operator
        add(state::found_lt, "", state::idle, token::_operator);
        add(state::found_lt, ":.,+-*/%~&|^!<>?@$", "+=", state::_operator);
        add(state::found_lt, "=", state::found_lt_eq, "+=");
        add(state::found_lt_eq, "", state::idle, token::_operator);
        add(state::found_lt_eq, ">+=", state::idle, token::_operator);
    }

    constexpr void add_comments() noexcept
    {
        if constexpr (Config.double_slash_line_comment or Config.c_block_comment) {
            // Add an indirection when starting comment with '/'.
            add(state::idle, "/+", state::comment_found_slash);
            add(state::comment_found_slash, "=/", state::found_slash);
        } else {
            add(state::idle, "/+=", state::found_slash);
        }

        if constexpr (Config.sgml_block_comment) {
            // Add an indirection when starting comment with '<'.
            add(state::idle, "<+", state::comment_found_lt);
            add(state::comment_found_less_than, "=<", state::found_lt);
        } else {
            add(state::idle, "<+=", state::found_lt);
        }

        if constexpr (Config.double_slash_line_comment) {
            add(state::comment_found_slash, '/', state::line_comment, no_capture);
        }

        if constexpr (Config.semicolon_line_comment) {
            add(state::idle, ';', state::line_comment, no_capture);
        } else {
            add(state::idle, ';', state::found_semicolon);
        }

        if constexpr (Config.hash_line_comment) {
            add(state::idle, '#', state::line_comment, no_capture);
        } else {
            add(state::idle, '#', state::found_hash);
        }

        if constexpr (Config.c_block_comment) {
            add(state::comment_found_slash, '*', state::block_comment, no_capture);
        }

        if constexpr (Config.sgml_comment) {
            add(state::comment_found_lt, '!', state::comment_found_lt_bang, no_capture);
            add(state::comment_found_lt_bang, '-', state::comment_found_lt_dash, no_capture);
            add(state::comment_found_lt_bang_dash, '-', state::block_comment, no_capture);
        }

        add(state::line_comment, any, state::line_comment);
        add(state::line_comment, "\n\f\v", state::idle, no_read, token::line_comment);

        add(state::block_comment, any, state::block_comment);

        if constexpr (Config.c_block_comment) {
            add(state::block_comment, '*', block_comment_found_star, no_capture);
            add(state::block_comment_found_star, any state::block_comment, no_read, '/');
            add(state::block_comment_found_star, '/', state::idle, no_capture, token::block_comment);
        }

        if constexpr (Config.sgml_block_comment) {
            add(state::block_comment, '-', block_comment_found_dash, no_capture);
            add(state::block_comment_found_dash, any, state::block_comment, no_read, '-');
            add(state::block_comment_found_dash, '-', state::block_comment_found_dash_dash, no_capture);
            add(state::block_comment_found_dash_dash, any, state::block_comment_found_dash_dash_fin0, no_read, '-');
            add(state::block_comment_found_dash_dash, '>', state::idle, no_capture, token::block_comment);
            add(state::block_comment_found_dash_dash_fin0, any, state::block_comment, no_read, '-');
        }
    }

    constexpr void add_separators() noexcept
    {
        add(state::idle, ",()[]{}", state::idle, token::separator);
        if constexpr (not Config.semicolon_line_comment) {
            add(state::idle, ';', state::idle, token::separator);
        }
    }

    constexpr void add_identifier() noexcept
    {
        add(idle, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_", identifier);
        add(identifier, identifier, token::identifier, no_read);
        add(identifier, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_0123456789", identifier);
    }

    [[nodiscard]] constexpr static size_t make_index(state_type from, char c) noexcept
    {
        auto c_ = char_cast<size_t>(c);
        return to_underlying(from) * 128 + index;
    }

    constexpr command_type& add(state_type from, char c, state_type to) noexcept
    {
        hilet i = make_index(from, c);
        auto& command = transition_table[i];
        command.next_state = to;
        command.char_to_capture = c;
        command.read = 1;
        command.clear_capture = 0;
        return command;
    }

    /** Add a state change.
     *
     * The attribute-arguments may be:
     * - token: The token will be emited and the capture-buffer is reset.
     * - no_read_tag: The current character will not advance.
     * - reset_tag: Reset the capture-buffer explicitely.
     * - char: The character to capture, the default is @a c, if no_capture no character is captured.
     *
     * @param from The current state.
     * @param c The current character.
     * @param to The next state.
     * @param first The first attribute-argument
     * @param args The rest of the attribute-arguments.
     */
    template<typename First, typename... Args>
    constexpr command_type& add(state_type from, char c, state_type to, First first, Args const&...args) noexcept
    {
        auto& command = add(from, c, to, args...);
        if constexpr (std::is_same_v<First, token>) {
            command.reset = 1;
            command.emit_token = first;

        } else if constexpr (std::is_same_v<First, no_read_tag>) {
            command.read = 0;

        } else if constexpr (std::is_same_v<First, no_capture_tag>) {
            command.char_to_capture = no_capture;

        } else if constexpr (std::is_same_v<First, reset_tag>) {
            command.reset = 1;

        } else if constexpr (std::is_same_v<First, char>) {
            command.char_to_capture = first;

        } else {
            hi_static_no_default();
        }

        return command;
    }

    template<size_t N, typename... Args>
    constexpr void add(state_type from, char (&str)[N], state_type to, Args const&...args) noexcept
    {
        for (auto i = 0_uz; i != N; ++i) {
            add(from, str[i], to, args...);
        }
    }

    template<typename... Args>
    constexpr void add(state_type from, state_type to, Args const&...args) noexcept
    {
        for (char c = 0; c != 127; ++c) {
            add(from, c, to, args...);
        }
    }
};

} // namespace detail

template<lexer_config Config>
constexpr auto lexer = detail::lexer<Config>();

}} // namespace hi::v1
