
#pragma once

#include "utility/module.hpp"
#include "unicode/module.hpp"
#include "char_maps/module.hpp"
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

enum class lexer_token_kind : uint8_t {
    none,
    error_unexepected_character,
    error_invalid_digit,
    error_incomplete_exponent,
    error_incomplete_string,
    error_incomplete_comment,
    error_after_lt_bang,
    integer,
    real,
    sstr,
    dstr,
    bstr,
    istr,
    color,
    lcomment,
    bcomment,
    ws,
    id,
    other
};

// clang-format off
constexpr auto lexer_token_kind_metadata = enum_metadata{
    lexer_token_kind::none, "none",
    lexer_token_kind::error_unexepected_character, "error:unexpected character",
    lexer_token_kind::error_invalid_digit, "error:invalid digit",
    lexer_token_kind::error_incomplete_exponent, "error:incomplete exponent",
    lexer_token_kind::error_incomplete_string, "error:incomplete string",
    lexer_token_kind::error_incomplete_comment, "error:incomplete comment",
    lexer_token_kind::error_after_lt_bang, "error:after_lt_bang",
    lexer_token_kind::integer, "integer",
    lexer_token_kind::real, "read",
    lexer_token_kind::sstr, "single-quote string",
    lexer_token_kind::dstr, "double-quote string",
    lexer_token_kind::bstr, "back-quote string",
    lexer_token_kind::istr, "ini string",
    lexer_token_kind::color, "color",
    lexer_token_kind::lcomment, "line comment",
    lexer_token_kind::bcomment, "block comment",
    lexer_token_kind::ws, "ws",
    lexer_token_kind::id, "id",
    lexer_token_kind::other, "other"
};
// clang-format on

struct lexer_token_type {
    std::vector<char> capture = {};
    size_t line_nr = 0;
    size_t column_nr = 0;
    lexer_token_kind kind = lexer_token_kind::none;

    constexpr lexer_token_type() noexcept = default;
    constexpr lexer_token_type(lexer_token_type const&) noexcept = default;
    constexpr lexer_token_type(lexer_token_type&&) noexcept = default;
    constexpr lexer_token_type& operator=(lexer_token_type const&) noexcept = default;
    constexpr lexer_token_type& operator=(lexer_token_type&&) noexcept = default;

    constexpr lexer_token_type(lexer_token_kind kind, std::string_view capture, size_t column_nr) noexcept :
        kind(kind), capture(), line_nr(0), column_nr(column_nr)
    {
        std::copy(capture.begin(), capture.end(), std::back_inserter(this->capture));
    }

    constexpr lexer_token_type(lexer_token_kind kind, std::string_view capture, size_t line_nr, size_t column_nr) noexcept :
        kind(kind), capture(), line_nr(line_nr), column_nr(column_nr)
    {
        std::copy(capture.begin(), capture.end(), std::back_inserter(this->capture));
    }

    [[nodiscard]] constexpr friend bool operator==(lexer_token_type const&, lexer_token_type const&) noexcept = default;

    [[nodiscard]] constexpr bool operator==(lexer_token_kind rhs) const noexcept
    {
        return kind == rhs;
    }

    [[nodiscard]] constexpr bool operator==(std::string_view rhs) const noexcept
    {
        return static_cast<std::string_view>(*this) == rhs;
    }

    [[nodiscard]] constexpr bool operator==(char rhs) const noexcept
    {
        return kind == lexer_token_kind::other and capture.size() == 1 and capture.front() == rhs;
    }

    constexpr operator std::string_view() const noexcept
    {
        return std::string_view{capture.data(), capture.size()};
    }

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

struct lexer_clear_tag {};
struct lexer_any_tag {};
struct lexer_advance_tag {};
struct lexer_capture_tag {};

class lexer_excluding_tag {
public:
    constexpr lexer_excluding_tag(std::string exclusions) noexcept : _exclusions(std::move(exclusions)) {}

    [[nodiscard]] constexpr bool contains(char c) const noexcept
    {
        return _exclusions.find(c) != _exclusions.npos;
    }

private:
    std::string _exclusions;
};

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

template<size_t N>
[[nodiscard]] constexpr lexer_excluding_tag lexer_excluding(char const (&exclusions)[N]) noexcept
{
    return lexer_excluding_tag{std::string(exclusions, N - 1)};
}

template<typename First, typename... Args>
[[nodiscard]] constexpr static bool _has_lexer_advance_tag_argument() noexcept
{
    if constexpr (std::is_same_v<First, lexer_advance_tag>) {
        return true;
    } else if constexpr (sizeof...(Args) == 0) {
        return false;
    } else {
        return _has_lexer_advance_tag_argument<Args...>();
    }
}

template<typename... Args>
[[nodiscard]] constexpr static bool has_lexer_advance_tag_argument() noexcept
{
    if constexpr (sizeof...(Args) == 0) {
        return false;
    } else {
        return _has_lexer_advance_tag_argument<Args...>();
    }
}

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

    /** Advance line_nr.
     */
    uint8_t advance_line : 1 = 0;

    /** Advance column_nr for a tab.
     */
    uint8_t advance_tab : 1 = 0;
};

template<lexer_config Config>
class lexer {
public:
    constexpr lexer() noexcept : _transition_table()
    {
        add(lexer_state_type::idle, '/', lexer_state_type::found_slash, lexer_advance, lexer_capture);
        add(lexer_state_type::idle, '<', lexer_state_type::found_lt, lexer_advance, lexer_capture);
        add(lexer_state_type::idle, '#', lexer_state_type::found_hash, lexer_advance, lexer_capture);
        add(lexer_state_type::idle, '.', lexer_state_type::found_dot, lexer_advance, lexer_capture);
        add(lexer_state_type::idle, '=', lexer_state_type::found_eq, lexer_advance, lexer_capture);
        add(lexer_state_type::idle, ':', lexer_state_type::found_colon, lexer_advance, lexer_capture);

        add(lexer_state_type::found_slash, lexer_any, lexer_state_type::idle, lexer_token_kind::other);
        add(lexer_state_type::found_lt, lexer_any, lexer_state_type::idle, lexer_token_kind::other);
        add(lexer_state_type::found_hash, lexer_any, lexer_state_type::idle, lexer_token_kind::other);
        add(lexer_state_type::found_dot, lexer_any, lexer_state_type::idle, lexer_token_kind::other);
        add(lexer_state_type::found_eq, lexer_any, lexer_state_type::idle, lexer_token_kind::other);
        add(lexer_state_type::found_colon, lexer_any, lexer_state_type::idle, lexer_token_kind::other);

        // Adds the starters "\"'`"
        add_string_literals();

        // Adds the starters "0123456789"
        add_number_literals();

        add_color_literal();
        add_comments();
        add_white_space();
        add_identifier();
        add_ini_assignment();

        add(lexer_state_type::idle,
            "~!@$%^&*()-+[]{}\\|,>?",
            lexer_state_type::idle,
            lexer_token_kind::other,
            lexer_capture,
            lexer_advance);

        // All unused entries of the idle state are unexpected characters.
        for (uint8_t i = 0; i != 128; ++i) {
            auto& command = get_command(lexer_state_type::idle, char_cast<char>(i));
            if (not command.assigned) {
                command.assigned = 1;
                command.advance = 1;
                // If there are actual null characters in the string then nothing gets captured.
                command.char_to_capture = char_cast<char>(i);
                command.emit_token = lexer_token_kind::error_unexepected_character;
                command.next_state = lexer_state_type::idle;
            }
        }
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
        constexpr iterator(lexer const *lexer, It first, ItEnd last) noexcept :
            _lexer(lexer), _first(first), _last(last), _it(first)
        {
            _cp = advance();
            do {
                _token.kind = parse_token();
            } while (Config.filter_white_space and _token.kind == lexer_token_kind::ws);
        }

        [[nodiscard]] constexpr lexer_token_type const& operator*() const noexcept
        {
            return _token;
        }

        constexpr iterator& operator++() noexcept
        {
            hi_axiom(*this != std::default_sentinel);
            do {
                _token.kind = parse_token();
            } while (Config.filter_white_space and _token.kind == lexer_token_kind::ws);
            return *this;
        }

        [[nodiscard]] constexpr bool operator==(std::default_sentinel_t) const noexcept
        {
            return _token.kind == lexer_token_kind::none;
        }

    private:
        lexer const *_lexer;
        It _first;
        ItEnd _last;
        It _it;
        char32_t _cp = 0;
        lexer_token_type _token;
        lexer_state_type _state = lexer_state_type::idle;
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

        [[nodiscard]] constexpr lexer_token_kind parse_token_unicode_identifier() noexcept
        {
            switch (ucd_get_lexical_class(_cp & 0x1f'ffff)) {
            case unicode_lexical_class::id_start:
            case unicode_lexical_class::id_continue:
                capture(_cp);
                advance_counters();
                _cp = advance();
                return lexer_token_kind::none;

            default:
                _state = lexer_state_type::idle;
                return lexer_token_kind::id;
            }
        }

        [[nodiscard]] constexpr lexer_token_kind parse_token_unicode_line_comment() noexcept
        {
            hilet cp_ = _cp & 0x1f'ffff;
            if (cp_ == U'\u0085' or cp_ == U'\u2028' or cp_ == U'\u2029') {
                _state = lexer_state_type::idle;
                advance_counters();
                _cp = advance();
                return lexer_token_kind::lcomment;

            } else {
                capture(_cp);
                advance_counters();
                _cp = advance();
                return lexer_token_kind::none;
            }
        }

        [[nodiscard]] constexpr lexer_token_kind parse_token_unicode_white_space() noexcept
        {
            if (ucd_get_lexical_class(_cp & 0x1f'ffff) == unicode_lexical_class::white_space) {
                capture(_cp);
                advance_counters();
                _cp = advance();
                return lexer_token_kind::none;

            } else {
                _state = lexer_state_type::idle;
                return lexer_token_kind::ws;
            }
        }

        [[nodiscard]] constexpr lexer_token_kind parse_token_unicode_idle() noexcept
        {
            switch (ucd_get_lexical_class(_cp & 0x1f'ffff)) {
            case unicode_lexical_class::id_start:
                _state = lexer_state_type::identifier;
                capture(_cp);
                advance_counters();
                _cp = advance();
                return lexer_token_kind::none;

            case unicode_lexical_class::white_space:
                _state = lexer_state_type::white_space;
                capture(_cp);
                advance_counters();
                _cp = advance();
                return lexer_token_kind::none;

            case unicode_lexical_class::syntax:
                _state = lexer_state_type::idle;
                capture(_cp);
                advance_counters();
                _cp = advance();
                return lexer_token_kind::other;

            default:
                capture(_cp);
                advance_counters();
                _cp = advance();
                return lexer_token_kind::error_unexepected_character;
            }
        }

        [[nodiscard]] hi_no_inline constexpr lexer_token_kind parse_token_unicode() noexcept
        {
            // Unicode by-pass.
            switch (_state) {
            case lexer_state_type::idle:
                return parse_token_unicode_idle();

            case lexer_state_type::white_space:
                return parse_token_unicode_white_space();

            case lexer_state_type::line_comment:
                return parse_token_unicode_line_comment();

            case lexer_state_type::identifier:
                return parse_token_unicode_identifier();

            case lexer_state_type::dqstring_literal:
            case lexer_state_type::sqstring_literal:
            case lexer_state_type::bqstring_literal:
            case lexer_state_type::block_comment:
                capture(_cp);
                advance_counters();
                _cp = advance();
                return lexer_token_kind::none;

            case lexer_state_type::ini_string:
                // Line-feeds will terminate an ini-string.
                if (_cp == U'\u0085' or _cp == U'\u2028' or _cp == U'\u2029') {
                    return lexer_token_kind::istr;
                } else {
                    capture(_cp);
                    advance_counters();
                    _cp = advance();
                    return lexer_token_kind::none;
                }

            default:
                // Most tokens are terminated when a non-ascii code-point is found.
                // Terminate these tokens as if we reached end-of-file.
                while (_state != lexer_state_type::idle) {
                    if (auto token_kind = process_command(); token_kind != lexer_token_kind::none) {
                        return token_kind;
                    }
                }
                return lexer_token_kind::none;
            }
        }

        [[nodiscard]] constexpr lexer_token_kind process_command(char c = '\0') noexcept
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

        [[nodiscard]] constexpr lexer_token_kind parse_token() noexcept
        {
            _token.line_nr = _line_nr;
            _token.column_nr = _column_nr;
            clear();

            while (_cp <= 0x7fff'ffff) {
                if (_cp <= 0x7f) {
                    if (auto token_kind = process_command(char_cast<char>(_cp)); token_kind != lexer_token_kind::none) {
                        return token_kind;
                    }

                } else {
                    auto emit_token = parse_token_unicode();
                    if (emit_token != lexer_token_kind::none) {
                        return emit_token;
                    }
                }
            }

            // Handle trailing state changes at end-of-file.
            while (_state != lexer_state_type::idle) {
                if (auto token_kind = process_command(); token_kind != lexer_token_kind::none) {
                    return token_kind;
                }
            }

            // We have finished parsing and there was no token captured.
            // For example when the end of file only contains white-space.
            return lexer_token_kind::none;
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

    constexpr void add_string_literal(
        char c,
        lexer_token_kind string_token,
        lexer_state_type string_literal,
        lexer_state_type string_literal_quote,
        lexer_state_type string_literal_escape) noexcept
    {
        add(lexer_state_type::idle, c, string_literal, lexer_advance);
        add(string_literal, lexer_any, lexer_state_type::idle, lexer_token_kind::error_incomplete_string);
        for (uint8_t i = 1; i != 128; ++i) {
            if (char_cast<char>(i) != c and char_cast<char>(i) != '\\') {
                add(string_literal, char_cast<char>(i), string_literal, lexer_advance, lexer_capture);
            }
        }

        if constexpr (Config.escape_by_quote_doubling) {
            // Don't capture the first quote.
            add(string_literal, c, string_literal_quote, lexer_advance);
            // If quote is not doubled, this is the end of the string.
            add(string_literal_quote, lexer_any, lexer_state_type::idle, string_token);
            // Capture one quote of a doubled quote.
            add(string_literal_quote, c, string_literal, lexer_advance, lexer_capture);
        } else {
            // Quote ends the string.
            add(string_literal, c, lexer_state_type::idle, lexer_advance, string_token);
        }

        // Make sure that lexer_any escaped character sequence stays inside the string literal.
        add(string_literal, '\\', string_literal_escape, lexer_advance, lexer_capture);
        add(string_literal_escape, lexer_any, lexer_state_type::idle, lexer_token_kind::error_incomplete_string);
        for (uint8_t i = 1; i != 128; ++i) {
            add(string_literal_escape, char_cast<char>(i), string_literal, lexer_advance, lexer_capture);
        }
    }

    constexpr void add_string_literals() noexcept
    {
        if constexpr (Config.has_single_quote_string_literal) {
            add_string_literal(
                '\'',
                lexer_token_kind::sstr,
                lexer_state_type::sqstring_literal,
                lexer_state_type::sqstring_literal_quote,
                lexer_state_type::sqstring_literal_escape);
        } else {
            add(lexer_state_type::idle, '\'', lexer_state_type::idle, lexer_token_kind::other, lexer_advance, lexer_capture);
        }

        if constexpr (Config.has_double_quote_string_literal) {
            add_string_literal(
                '"',
                lexer_token_kind::dstr,
                lexer_state_type::dqstring_literal,
                lexer_state_type::dqstring_literal_quote,
                lexer_state_type::dqstring_literal_escape);
        } else {
            add(lexer_state_type::idle, '"', lexer_state_type::idle, lexer_token_kind::other, lexer_advance, lexer_capture);
        }

        if constexpr (Config.has_back_quote_string_literal) {
            add_string_literal(
                '`',
                lexer_token_kind::bstr,
                lexer_state_type::btstring_literal,
                lexer_state_type::btstring_literal_quote,
                lexer_state_type::btstring_literal_escape);
        } else {
            add(lexer_state_type::idle, '`', lexer_state_type::idle, lexer_token_kind::other, lexer_advance, lexer_capture);
        }
    }

    constexpr void add_number_literals() noexcept
    {
        add(lexer_state_type::idle, "0", lexer_state_type::zero, lexer_advance, lexer_capture);
        add(lexer_state_type::idle, "123456789", lexer_state_type::dec_integer, lexer_advance, lexer_capture);

        add(lexer_state_type::zero, lexer_any, lexer_state_type::idle, lexer_token_kind::integer);
        add(lexer_state_type::zero, "bB", lexer_state_type::bin_integer, lexer_advance, lexer_capture);
        add(lexer_state_type::zero, "oO", lexer_state_type::oct_integer, lexer_advance, lexer_capture);
        add(lexer_state_type::zero, "dD", lexer_state_type::dec_integer, lexer_advance, lexer_capture);
        add(lexer_state_type::zero, "xX", lexer_state_type::hex_integer, lexer_advance, lexer_capture);

        if constexpr (Config.zero_starts_octal) {
            add(lexer_state_type::zero, "01234567", lexer_state_type::oct_integer, lexer_advance, lexer_capture);
            add(lexer_state_type::zero, "89", lexer_state_type::idle, lexer_token_kind::error_invalid_digit);
        } else {
            add(lexer_state_type::zero, "0123456789", lexer_state_type::dec_integer, lexer_advance, lexer_capture);
        }

        // binary-integer
        add(lexer_state_type::bin_integer, lexer_any, lexer_state_type::idle, lexer_token_kind::integer);
        add(lexer_state_type::bin_integer, "01", lexer_state_type::bin_integer, lexer_advance, lexer_capture);
        add(lexer_state_type::bin_integer, "23456789", lexer_state_type::idle, lexer_token_kind::error_invalid_digit);

        // octal-integer
        add(lexer_state_type::oct_integer, lexer_any, lexer_state_type::idle, lexer_token_kind::integer);
        add(lexer_state_type::oct_integer, "01234567", lexer_state_type::oct_integer, lexer_advance, lexer_capture);
        add(lexer_state_type::oct_integer, "89", lexer_state_type::idle, lexer_token_kind::error_invalid_digit);

        // decimal-integer
        add(lexer_state_type::dec_integer, lexer_any, lexer_state_type::idle, lexer_token_kind::integer);
        add(lexer_state_type::dec_integer, "0123456789", lexer_state_type::dec_integer, lexer_advance, lexer_capture);
        add(lexer_state_type::dec_integer, ".", lexer_state_type::dec_float, lexer_advance, lexer_capture);
        add(lexer_state_type::dec_integer, "eE", lexer_state_type::dec_sign_exponent, lexer_advance, lexer_capture);

        // hexadecimal-integer
        add(lexer_state_type::hex_integer, lexer_any, lexer_state_type::idle, lexer_token_kind::integer);
        add(lexer_state_type::hex_integer, "0123456789abcdefABCDEF", lexer_state_type::hex_integer, lexer_advance, lexer_capture);
        add(lexer_state_type::hex_integer, ".", lexer_state_type::hex_float, lexer_advance, lexer_capture);
        add(lexer_state_type::hex_integer, "pP", lexer_state_type::hex_sign_exponent, lexer_advance, lexer_capture);

        // decimal-float
        add(lexer_state_type::found_dot, "0123456789eE", lexer_state_type::dec_float);
        add(lexer_state_type::dec_float, lexer_any, lexer_state_type::idle, lexer_token_kind::real);
        add(lexer_state_type::dec_float, "0123456789", lexer_state_type::dec_float, lexer_advance, lexer_capture);
        add(lexer_state_type::dec_float, "eE", lexer_state_type::dec_sign_exponent, lexer_advance, lexer_capture);
        add(lexer_state_type::dec_sign_exponent, lexer_any, lexer_state_type::idle, lexer_token_kind::error_incomplete_exponent);
        add(lexer_state_type::dec_sign_exponent, "0123456789", lexer_state_type::dec_exponent_more, lexer_advance, lexer_capture);
        add(lexer_state_type::dec_sign_exponent, "+-", lexer_state_type::dec_exponent, lexer_advance, lexer_capture);
        add(lexer_state_type::dec_exponent, lexer_any, lexer_state_type::idle, lexer_token_kind::error_incomplete_exponent);
        add(lexer_state_type::dec_exponent, "0123456789", lexer_state_type::dec_exponent_more, lexer_advance, lexer_capture);
        add(lexer_state_type::dec_exponent_more, lexer_any, lexer_state_type::idle, lexer_token_kind::real);
        add(lexer_state_type::dec_exponent_more, "0123456789", lexer_state_type::dec_exponent_more, lexer_advance, lexer_capture);

        // hexadecimal-float
        add(lexer_state_type::hex_float, lexer_any, lexer_state_type::idle, lexer_token_kind::real);
        add(lexer_state_type::hex_float, "0123456789abcdefABCDEF", lexer_state_type::hex_float, lexer_advance, lexer_capture);
        add(lexer_state_type::hex_float, "pP", lexer_state_type::hex_sign_exponent, lexer_advance, lexer_capture);
        add(lexer_state_type::hex_sign_exponent, lexer_any, lexer_state_type::idle, lexer_token_kind::error_incomplete_exponent);
        add(lexer_state_type::hex_sign_exponent,
            "0123456789abcdefABCDEF",
            lexer_state_type::hex_exponent_more,
            lexer_advance,
            lexer_capture);
        add(lexer_state_type::hex_sign_exponent, "+-", lexer_state_type::hex_exponent, lexer_advance, lexer_capture);
        add(lexer_state_type::hex_exponent, lexer_any, lexer_state_type::idle, lexer_token_kind::error_incomplete_exponent);
        add(lexer_state_type::hex_exponent,
            "0123456789abcdefABCDEF",
            lexer_state_type::hex_exponent_more,
            lexer_advance,
            lexer_capture);
        add(lexer_state_type::hex_exponent_more, lexer_any, lexer_state_type::idle, lexer_token_kind::real);
        add(lexer_state_type::hex_exponent_more,
            "0123456789abcdefABCDEF",
            lexer_state_type::hex_exponent_more,
            lexer_advance,
            lexer_capture);

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
    }

    constexpr void add_color_literal() noexcept
    {
        if constexpr (Config.has_color_literal) {
            add(lexer_state_type::found_hash,
                "0123456789abcdefABCDEF",
                lexer_state_type::color_literal,
                lexer_clear,
                lexer_capture,
                lexer_advance);
            add(lexer_state_type::color_literal, lexer_any, lexer_state_type::idle, lexer_token_kind::color);
            add(lexer_state_type::color_literal,
                "0123456789abcdefABCDEF",
                lexer_state_type::color_literal,
                lexer_advance,
                lexer_capture);
        }
    }

    constexpr void add_ini_assignment() noexcept
    {
        if constexpr (Config.equal_is_ini_assignment) {
            // Ignore white-space
            add(lexer_state_type::found_eq, " \t", lexer_state_type::found_eq, lexer_advance);
            add(lexer_state_type::found_eq,
                "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_",
                lexer_state_type::ini_string,
                lexer_token_kind::other);
        }

        if constexpr (Config.colon_is_ini_assignment) {
            // Ignore white-space
            add(lexer_state_type::found_colon, " \t", lexer_state_type::found_colon, lexer_advance);
            add(lexer_state_type::found_colon,
                "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_",
                lexer_state_type::ini_string,
                lexer_token_kind::other);
        }

        add(lexer_state_type::ini_string, lexer_any, lexer_state_type::idle, lexer_token_kind::istr);
        add(lexer_state_type::ini_string,
            lexer_excluding("\n\v\f\r\0"),
            lexer_state_type::ini_string,
            lexer_advance,
            lexer_capture);
        add(lexer_state_type::ini_string, '\r', lexer_state_type::ini_string, lexer_advance);
    }

    constexpr void add_comments() noexcept
    {
        if constexpr (Config.has_double_slash_line_comment) {
            add(lexer_state_type::found_slash, '/', lexer_state_type::line_comment, lexer_clear, lexer_advance);
        }

        if constexpr (Config.has_semicolon_line_comment) {
            add(lexer_state_type::idle, ';', lexer_state_type::line_comment, lexer_advance);
        } else {
            add(lexer_state_type::idle, ';', lexer_state_type::idle, lexer_token_kind::other, lexer_capture, lexer_advance);
        }

        if constexpr (Config.has_hash_line_comment) {
            add(lexer_state_type::found_hash,
                lexer_excluding("\0"),
                lexer_state_type::line_comment,
                lexer_clear,
                lexer_advance,
                lexer_capture);
        }

        if constexpr (Config.has_c_block_comment) {
            add(lexer_state_type::found_slash, '*', lexer_state_type::block_comment, lexer_advance, lexer_clear);
        }

        if constexpr (Config.has_sgml_block_comment) {
            add(lexer_state_type::found_lt, '!', lexer_state_type::found_lt_bang, lexer_advance);
            add(lexer_state_type::found_lt_bang, lexer_any, lexer_state_type::idle, lexer_token_kind::error_after_lt_bang);
            add(lexer_state_type::found_lt_bang, '-', lexer_state_type::found_lt_bang_dash, lexer_advance);
            add(lexer_state_type::found_lt_bang_dash, lexer_any, lexer_state_type::idle, lexer_token_kind::error_after_lt_bang);
            add(lexer_state_type::found_lt_bang_dash, '-', lexer_state_type::block_comment, lexer_advance);
        }

        add(lexer_state_type::line_comment, lexer_any, lexer_state_type::idle, lexer_token_kind::lcomment);
        add(lexer_state_type::line_comment,
            lexer_excluding("\r\n\f\v\0"),
            lexer_state_type::line_comment,
            lexer_advance,
            lexer_capture);

        add(lexer_state_type::line_comment, '\r', lexer_state_type::line_comment, lexer_advance);
        add(lexer_state_type::line_comment, "\n\f\v", lexer_state_type::idle, lexer_advance, lexer_token_kind::lcomment);

        add(lexer_state_type::block_comment, lexer_any, lexer_state_type::idle, lexer_token_kind::error_incomplete_comment);

        static_assert(Config.has_c_block_comment == 0 or Config.has_sgml_block_comment == 0);

        if constexpr (Config.has_c_block_comment) {
            add(lexer_state_type::block_comment,
                lexer_excluding("*\0"),
                lexer_state_type::block_comment,
                lexer_advance,
                lexer_capture);
            add(lexer_state_type::block_comment, '*', lexer_state_type::block_comment_found_star, lexer_advance);
            add(lexer_state_type::block_comment_found_star, lexer_any, lexer_state_type::block_comment, '*');
            add(lexer_state_type::block_comment_found_star,
                '/',
                lexer_state_type::idle,
                lexer_advance,
                lexer_token_kind::bcomment);

        } else if constexpr (Config.has_sgml_block_comment) {
            add(lexer_state_type::block_comment,
                lexer_excluding("-\0"),
                lexer_state_type::block_comment,
                lexer_advance,
                lexer_capture);
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
                lexer_token_kind::bcomment);
        }
    }

    constexpr void add_white_space() noexcept
    {
        add(lexer_state_type::idle, '\r', lexer_state_type::white_space, lexer_advance);
        add(lexer_state_type::idle, " \n\t\v\f", lexer_state_type::white_space, lexer_advance, lexer_capture);
        add(lexer_state_type::white_space, lexer_any, lexer_state_type::idle, lexer_token_kind::ws);
        add(lexer_state_type::white_space, '\r', lexer_state_type::white_space, lexer_advance);
        add(lexer_state_type::white_space, " \n\t\v\f", lexer_state_type::white_space, lexer_advance, lexer_capture);
    }

    constexpr void add_identifier() noexcept
    {
        add(lexer_state_type::idle,
            "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_",
            lexer_state_type::identifier,
            lexer_advance,
            lexer_capture);
        add(lexer_state_type::identifier, lexer_any, lexer_state_type::idle, lexer_token_kind::id);
        add(lexer_state_type::identifier,
            "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_0123456789",
            lexer_state_type::identifier,
            lexer_advance,
            lexer_capture);
    }

    constexpr lexer_command_type& _add(lexer_state_type from, char c, lexer_state_type to) noexcept
    {
        auto& command = get_command(from, c);
        command.next_state = to;
        command.char_to_capture = '\0';
        command.advance = 0;
        command.advance_line = 0;
        command.advance_tab = 0;
        command.clear = 0;
        command.emit_token = lexer_token_kind::none;
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
    _add(lexer_state_type from, char c, lexer_state_type to, First first, Args const&...args) noexcept
    {
        auto& command = _add(from, c, to, args...);
        if constexpr (std::is_same_v<First, lexer_token_kind>) {
            command.emit_token = first;

        } else if constexpr (std::is_same_v<First, lexer_advance_tag>) {
            command.advance = 1;
            if (c == '\n' or c == '\v' or c == '\f') {
                command.advance_line = 1;
            } else if (c == '\t') {
                command.advance_tab = 1;
            }

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
    constexpr void add(lexer_state_type from, char c, lexer_state_type to, Args const&...args) noexcept
    {
        auto& command = _add(from, c, to, args...);
        hi_assert(not command.assigned, "Overwriting an already assigned state:char combination.");
        command.assigned = true;
    }

    template<typename... Args>
    constexpr void add(lexer_state_type from, std::string_view str, lexer_state_type to, Args const&...args) noexcept
    {
        for (auto c : str) {
            auto& command = _add(from, c, to, args...);
            hi_assert(not command.assigned, "Overwriting an already assigned state:char combination.");
            command.assigned = true;
        }
    }

    template<typename... Args>
    constexpr void add(lexer_state_type from, lexer_any_tag, lexer_state_type to, Args const&...args) noexcept
    {
        static_assert(not has_lexer_advance_tag_argument<Args...>(), "lexer_any should not advance");

        for (uint8_t c = 0; c != 128; ++c) {
            hilet& command = _add(from, char_cast<char>(c), to, args...);
            hi_assert(not command.assigned, "lexer_any should be added first to a state");
        }
    }

    template<typename... Args>
    constexpr void
    add(lexer_state_type from, lexer_excluding_tag const& exclusions, lexer_state_type to, Args const&...args) noexcept
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

template<typename CharT>
struct std::formatter<hi::lexer_token_type, CharT> : std::formatter<std::string, CharT> {
    auto format(hi::lexer_token_type const& t, auto& fc)
    {
        return std::formatter<std::string, CharT>::format(
            std::format(
                "{} \"{}\" {}:{}",
                hi::lexer_token_kind_metadata[t.kind],
                static_cast<std::string_view>(t),
                t.line_nr,
                t.column_nr),
            fc);
    }
};
