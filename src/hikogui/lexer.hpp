
#pragma once

namespace hi {
inline namespace v1 {

struct lexer_config {
    /** A zero starts in octal number.
     *
     * By default a zero starts a decimal number, but some languages
     * like C and C++ start an octal number with zero.
     */
    bool zero_starts_octal = false;

    /** The character used to seperate groups of numbers.
     *
     * This character is the character that will be ignored by a language
     * if it apears in a integer or floating point literal.
     *
     * For C and C++ this is the quote character, some other languages use
     * an underscore. If the language does not support group separator set this to no_capture.
     */
    char number_group_separator = no_capture;

    /** Escaping quotes within a string may be done using quote doubling.
     */
    bool escape_by_quote_doubling = false;
};

namespace detail {

template<lexer_config Config>
class lexer {
public:
    enun class token_type: uint8_t {
        none
    };

    constexpr static auto zero_starts_octal = Config.zero_starts_octal;
    constexpr static auto number_group_separator = Config.number_group_separator;

private:
    enum class state_type: uint8_t {
        idle,
        zero,
        bin_integer,
        oct_integer,
        dec_integer,
        hex_integer,

        _size
    };

    struct clear_tag{};
    struct no_read_tag{};
    struct no_capture_tag{};

    constexpr static char no_capture = no_capture_tag{};
    constexpr static auto no_read = no_read_tag{};
    constexpr static auto clear = clear_tag{};

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
         * If this is token_type::none then no token is emitted.
         */
        token_type emit_token = token_type::none;

        /** The char to capture.
         * If this is nul, then nothing is captured.
         */
        char char_to_capture = no_capture;

        /** Clear the capture buffer.
         */
        uint8_t clear: 1 = 0;

        /** Read a character, and advance the iterator.
         */
        uint8_t read: 1 = 0;
    };

    /** A array of commands, one for each state and character.
     * The array is in state-major order.
     */
    using transition_table_type = std::array<command_type,to_underlying(state_type::_size) * 128>;

    transition_table_type _transition_table;

public:

    constexpr lexer() noexcept : _transition_table()
    {
        add_literal_numbers();
        add_literal_string(
            '\'', token_type::sqstring_literal, sqstring_literal, sqstring_literal_quote, sqstring_literal_escape, sqstring_literal_escape_finish);
        add_literal_string(
            '"', token_type::dqstring_literal, dqstring_literal, dqstring_literal_quote, dqstring_literal_escape, dqstring_literal_escape_finish);
        add_literal_string(
            '`', token_type::btstring_literal, btstring_literal, btstring_literal_quote, btstring_literal_escape, btstring_literal_escape_finish);
        add_line_comment();
        add_block_comment();
    }

    constexpr void add_literal_numbers() noexcept
    {
        add(idle, "0", zero);
        add(idle, "123456789", dec_integer);

        add(zero, idle, token_type::integer_literal, no_read);
        add(zero, "bB", bin_integer);
        add(zero, "oO", oct_integer);
        add(zero, "dD", dec_integer);
        add(zero, "xX", hex_integer);
        if constexpr (zero_starts_octal) {
            add(zero, "01234567", oct_integer);
        } else {
            add(zero, "0123456789", dec_integer);
        }

        if constexpr (number_group_separator != no_capture) {
            // Don't capture number-group-separators.
            add(zero, number_group_separator, zero_starts_octal ? oct_integer : dec_integer, no_capture);
            add(bin_integer, number_group_separator, bin_integer, no_capture);
            add(oct_integer, number_group_separator, oct_integer, no_capture);
            add(dec_integer, number_group_separator, dec_integer, no_capture);
            add(hex_integer, number_group_separator, hex_integer, no_capture);
            add(dec_float, number_group_separator, dec_integer, no_capture);
            add(hex_float, number_group_separator, dec_integer, no_capture);
            add(dec_exponent, number_group_separator, dec_integer, no_capture);
            add(hex_exponent, number_group_separator, dec_integer, no_capture);
        }

        // binary-integer
        add(bin_integer, idle, token_type::integer_literal, no_read);
        add(bin_integer, "01", bin_integer);

        // octal-integer
        add(oct_integer, idle, token_type::integer_literal, no_read);
        add(oct_integer, "01234567", oct_integer);

        // decimal-integer
        add(dec_integer, idle, token_type::integer_literal, no_read);
        add(dec_integer, "0123456789", dec_integer);
        add(dec_integer, ".", dec_float);
        add(dec_integer, "eE", dec_sign_exponent);

        // hexadecimal-integer
        add(hex_integer, idle, token_type::integer_literal, no_read);
        add(hex_integer, "0123456789abcdefABCDEF", hex_integer);
        add(hex_integer, ".", hex_float);
        add(hex_integer, "pP", hex_sign_exponent);

        // decimal-float
        add(dec_float, idle, token_type::float_literal, no_read);
        add(dec_float, "0123456789", dec_float);
        add(dec_float, "eE", dec_sign_exponent);
        add(dec_sign_exponent, idle, token_type::error_missing_exponent_number, no_read);
        add(dec_sign_exponent, "0123456789", dec_exponent);
        add(dec_sign_exponent, "+-", dec_exponent);
        add(dec_exponent, idle, token_type::float_literal, no_read);
        add(dec_exponent, "0123456789", dec_exponent);

        // hexadecimal-float
        add(hex_float, idle, token_type::float_literal, no_read);
        add(hex_float, "0123456789abcdefABCDEF", hex_float);
        add(hex_float, "pP", hex_sign_exponent);
        add(hex_sign_exponent, idle, token_type::error_missing_exponent_number, no_read);
        add(hex_sign_exponent, "0123456789abcdefABCDEF", hex_exponent);
        add(hex_sign_exponent, "+-", hex_exponent);
        add(hex_exponent, idle, token_type::float_literal, no_read);
        add(hex_exponent, "0123456789abcdefABCDEF", hex_exponent);
    }

    constexpr void add_literal_string(
        char c,
        token_type token,
        state_type literal,
        state_type literal_quote,
        state_type literal_escape,
        state_type literal_escape_finish) noexcept
    {
        add(idle, c, literal, no_capture);
        add(literal, literal);
        if (escape_by_quote_doubling) {
            add(literal, '"', literal_quote, no_capture);
            add(literal_quote, idle, token_type::string_literal, no_capture);
            add(literal_quote, '"', literal)
        } else {
            add(literal, '"', idle, token_type::string_literal, no_capture);
        }
        add(literal, '\\', literal_escape, no_capture);
        add(literal_escape, literal_escape_finish, '\\', no_read);
        add(literal_escape, '"', literal);
        add(literal_escape, '\'', literal);
        add(literal_escape, '?', literal);
        add(literal_escape, 'a', literal, '\a');
        add(literal_escape, 'b', literal, '\b');
        add(literal_escape, 'f', literal, '\f');
        add(literal_escape, 'n', literal, '\n');
        add(literal_escape, 'r', literal, '\r');
        add(literal_escape, 't', literal, '\t');
        add(literal_escape, 'v', literal, '\v');
        add(literal_escape_finish, literal);
    }

    constexpr void add_comment() noexcept
    {
        add(idle, '/', slash, no_read, no_capture);
        add(slash, slash_finish, token_type::op, no_read);
        add(slash_finish, idle, token_type::op);

        if (cpp_comment) {
            add(slash, '/', line_comment, no_capture);
        }

        if (c_comment) {
            add(slash, '*', slash_star_block_comment, no_capture);
            add(slash_star_block_comment, slash_star_block_comment);
            add(slash_star_block_comment, '*', slash_star_star, no_capture);
            add(slash_star_star, slash_start_block_comment, '*', no_read);
            add(slash_star_star, '/', idle, no_capture);
        }

        if (sgml_comment) {
            add(slash, '<', slash_star_block_comment, no_capture);
            add(slash_star_block_comment, slash_star_block_comment);
            add(slash_star_block_comment, '*', slash_star_star, no_capture);
            add(slash_star_star, slash_start_block_comment, '*', no_read);
            add(slash_star_star, '/', idle, no_capture);
        }

        if (semicolon_starts_comment) {
            add(idle, ';', line_comment, no_capture);
        }

        if (hash_starts_comment) {
            add(idle, '#', line_comment, no_capture);
        }

        add(line_comment, line_comment);
        add(line_comment, "\n\f\v", idle, token_type::comment, no_read);
    }

    constexpr void add_identifier() noexcept
    {
        add(idle, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_", identifier);
        add(identifier, identifier, token_type::identigier, no_read);
        add(identifier, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_0123456789", identifier);
    }

    [[nodiscard]] constexpr static size_t make_index(state_type from, char c) noexcept
    {
        auto c_ = char_cast<size_t>(c);
        return to_underlying(from) * 128 + index;
    }

    constexpr command_type &add(state_type from, char c, state_type to) noexcept
    {
        hilet i = make_index(from, c);
        auto &command = transition_table[i];
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
    constexpr command_type &add(state_type from, char c, state_type to, First first, Args const &...args) noexcept
    {
        auto &command = add(from, c, to, args...);
        if constexpr (std::is_same_v<First, token_type>) {
            command.reset = 1;
            command.emit_token = first;

        } else if constexpr (std::is_same_v<First, no_read_tag) {
            command.read = 0;

        } else if constexpr (std::is_same_v<First, no_capture_tag) {
            command.char_to_capture = no_capture;

        } else if constexpr (std::is_same_v<First, reset_tag) {
            command.reset = 1;

        } else if constexpr (std::is_same_v<First, char) {
            command.char_to_capture = first;

        } else {
            hi_static_no_default();
        }

        return command;
    }

    template<size_t N, typename... Args>
    constexpr void add(state_type from, char (&str)[N], state_type to, Args const &...args) noexcept
    {
        for (auto i = 0_uz; i != N; ++i) {
            add(from, str[i], to, args...);
        }
    }

    template<typename... Args>
    constexpr void add(state_type from, state_type to, Args const &...args) noexcept
    {
        for (char c = 0; c != 127; ++c) {
            add(from, c, to, args...);
        }
    }
};

} // detail

template<lexer_config Config>
constexpr auto lexer = detail::lexer<Config>();

}}

