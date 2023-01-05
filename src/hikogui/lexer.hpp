
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
     * an underscore. If the language does not support group separator set this to '\0'.
     */
    char number_group_separator = '\0';
};

namespace detail {

template<lexer_config Config>
class lexer {
public:
    enun class token_type: uint8_t {
        none
    };

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
        token_type token_to_emit = token_type::none;

        /** The char to capture.
         * If this is nul, then nothing is captured.
         */
        char char_to_capture = '\0';

        /** Clear the capture buffer.
         */
        uint8_t clear_capture: 1 = 0;

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
        //
        // Literal numbers
        //
        add(idle, "0", zero, "+");
        add(idle, "123456789", dec_integer, "+");

        add(zero, "bB", bin_integer);
        add(zero, "oO", oct_integer);
        add(zero, "dD", dec_integer);
        add(zero, "xX", hex_integer);
        if constexpr (Config.zero_starts_octal) {
            add(zero, "01234567", oct_integer, "+");
        } else {
            add(zero, "0123456789", dec_integer, "+");
        }
        if constexpr (Config.number_group_separator != '\0') {
            add(zero, Config.number_group_separator, Config.zero_starts_octal ? oct_integer : dec_integer);
        }
        add(zero, " \n\r\t", idle, "!", Config.zero_starts_octal ? oct_integer_token : dec_integer_token);
    }

};

} // detail

template<lexer_config Config>
constexpr auto lexer = detail::lexer<Config>();

}}

