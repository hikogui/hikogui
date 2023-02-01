// Copyright Take Vos 2019-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "tokenizer.hpp"

namespace hi::inline v1 {

enum class tokenizer_state_t : uint8_t {
    Initial,
    Name,
    MinusOrPlus, // Could be the start of a number, or an operator starting with '+' or '-'.
    Zero, // Could be part of a number with a base.
    Dot, // Could be the start of a floating point number, or the '.' operator.
    Number, // Could be some kind of number without a base.
    DashAfterNumber, // Could be a date.
    ColonAfterNumber, // Could be a time.
    DashAfterInteger, // Integer followed by a operator starting with '-'
    ColonAfterInteger, // Integer followed by a operator starting with ':'
    Float,
    Date,
    Time,
    Quote,
    QuoteString,
    QuoteStringEscape,
    DQuote, // Could be the start of a string, an empty string, or block string.
    DoubleDQuote, // Is an empty string or a block string.
    DQuoteString,
    DQuoteStringEscape,
    BlockString,
    BlockStringDQuote,
    BlockStringDoubleDQuote,
    BlockStringCaptureDQuote,
    BlockStringEscape,
    Slash, // Could be the start of a LineComment, BlockComment, or an operator.
    LineComment,
    BlockComment,
    BlockCommentMaybeEnd, // Found a '*' possibly end of comment.
    OperatorFirstChar,
    OperatorSecondChar,
    OperatorThirdChar,
    ColonOperatorSecondChar,
    Sentinal
};
constexpr std::size_t NR_TOKENIZER_STATES = static_cast<std::size_t>(tokenizer_state_t::Sentinal);

enum class tokenizer_action_t : uint8_t {
    Idle = 0x00,
    Capture = 0x01, // Capture this character.
    Start = 0x02, // Start the capture queue.
    Read = 0x04, // Read next character, before processing next state. This will also advance the location.
    Found = 0x08, // Token Found.
    Tab = 0x10, // Move the location modulo 8 to the right.
    LineFeed = 0x20, // Move to the next line.
    Poison = 0x80, // Cleared.
};

constexpr uint16_t get_offset(tokenizer_state_t state, char c = '\0') noexcept
{
    return (wide_cast<uint16_t>(to_underlying(state)) << 8) | char_cast<uint8_t>(c);
}

constexpr tokenizer_action_t operator|(tokenizer_action_t lhs, tokenizer_action_t rhs)
{
    return static_cast<tokenizer_action_t>(to_underlying(lhs) | to_underlying(rhs));
}

constexpr tokenizer_action_t operator|(tokenizer_action_t lhs, char rhs)
{
    switch (rhs) {
    case '\n': return lhs | tokenizer_action_t::LineFeed;
    case '\f': return lhs | tokenizer_action_t::LineFeed;
    case '\t': return lhs | tokenizer_action_t::Tab;
    default: return lhs | tokenizer_action_t::Idle;
    }
}

constexpr bool operator>=(tokenizer_action_t lhs, tokenizer_action_t rhs)
{
    return (to_underlying(lhs) & to_underlying(rhs)) == to_underlying(rhs);
}

struct tokenizer_transition_t {
    tokenizer_state_t next;
    tokenizer_action_t action;
    char c;
    tokenizer_name_t name;

    constexpr tokenizer_transition_t(
        char c,
        tokenizer_state_t next = tokenizer_state_t::Initial,
        tokenizer_action_t action = tokenizer_action_t::Idle,
        tokenizer_name_t name = tokenizer_name_t::NotAssigned) :
        next(next), action(action), c(c), name(name)
    {
    }

    constexpr tokenizer_transition_t() :
        next(tokenizer_state_t::Initial), action(tokenizer_action_t::Idle), c('\0'), name(tokenizer_name_t::NotAssigned)
    {
    }
};

constexpr std::array<tokenizer_transition_t, 256> calculateTransitionTable_Name()
{
    std::array<tokenizer_transition_t, 256> r{};

    for (uint16_t i = 0; i < r.size(); i++) {
        hilet c = char_cast<char>(i);
        tokenizer_transition_t transition = {c};

        if (is_name_next(c) || c == '-') {
            transition.next = tokenizer_state_t::Name;
            transition.action = tokenizer_action_t::Read | tokenizer_action_t::Capture;
        } else {
            transition.next = tokenizer_state_t::Initial;
            transition.action = tokenizer_action_t::Found;
            transition.name = tokenizer_name_t::Name;
        }

        r[i] = transition;
    }
    return r;
}

constexpr std::array<tokenizer_transition_t, 256> calculateTransitionTable_MinusOrPlus()
{
    std::array<tokenizer_transition_t, 256> r{};

    for (uint16_t i = 0; i < r.size(); i++) {
        hilet c = char_cast<char>(i);
        tokenizer_transition_t transition = {c};

        if (c == '0') {
            transition.next = tokenizer_state_t::Zero;
            transition.action = tokenizer_action_t::Read | tokenizer_action_t::Capture;
        } else if (is_digit(c) || c == '.') {
            transition.next = tokenizer_state_t::Number;
        } else {
            transition.next = tokenizer_state_t::OperatorSecondChar;
        }

        r[i] = transition;
    }
    return r;
}

constexpr std::array<tokenizer_transition_t, 256> calculateTransitionTable_Dot()
{
    std::array<tokenizer_transition_t, 256> r{};

    for (uint16_t i = 0; i < r.size(); i++) {
        hilet c = char_cast<char>(i);
        tokenizer_transition_t transition = {c};

        if (is_digit(c)) {
            transition.next = tokenizer_state_t::Float;
        } else {
            transition.next = tokenizer_state_t::Initial;
            transition.action = tokenizer_action_t::Found;
            transition.name = tokenizer_name_t::Operator;
        }

        r[i] = transition;
    }
    return r;
}

constexpr std::array<tokenizer_transition_t, 256> calculateTransitionTable_Zero()
{
    std::array<tokenizer_transition_t, 256> r{};

    for (uint16_t i = 0; i < r.size(); i++) {
        hilet c = char_cast<char>(i);
        tokenizer_transition_t transition = {c};

        if (c == 'x' || c == 'X' || c == 'd' || c == 'D' || c == 'o' || c == 'O' || c == 'b' || c == 'B') {
            transition.next = tokenizer_state_t::Number;
            transition.action = tokenizer_action_t::Read | tokenizer_action_t::Capture;
        } else {
            transition.next = tokenizer_state_t::Number;
        }

        r[i] = transition;
    }
    return r;
}

constexpr std::array<tokenizer_transition_t, 256> calculateTransitionTable_Number()
{
    std::array<tokenizer_transition_t, 256> r{};

    for (uint16_t i = 0; i < r.size(); i++) {
        hilet c = char_cast<char>(i);
        tokenizer_transition_t transition = {c};

        if (is_digit(c)) {
            transition.next = tokenizer_state_t::Number;
            transition.action = tokenizer_action_t::Read | tokenizer_action_t::Capture;
        } else if (c == '_' || c == '\'') {
            transition.next = tokenizer_state_t::Number;
            transition.action = tokenizer_action_t::Read;
        } else if (c == '.') {
            transition.next = tokenizer_state_t::Float;
            transition.action = tokenizer_action_t::Read | tokenizer_action_t::Capture;
        } else if (c == '-') {
            transition.next = tokenizer_state_t::DashAfterNumber;
            transition.action = tokenizer_action_t::Read;
        } else if (c == ':') {
            transition.next = tokenizer_state_t::ColonAfterNumber;
            transition.action = tokenizer_action_t::Read;
        } else {
            transition.next = tokenizer_state_t::Initial;
            transition.action = tokenizer_action_t::Found;
            transition.name = tokenizer_name_t::IntegerLiteral;
        }

        r[i] = transition;
    }
    return r;
}

constexpr std::array<tokenizer_transition_t, 256> calculateTransitionTable_DashAfterNumber()
{
    std::array<tokenizer_transition_t, 256> r{};

    for (uint16_t i = 0; i < r.size(); i++) {
        hilet c = char_cast<char>(i);
        tokenizer_transition_t transition = {'-'};

        if (is_digit(c)) {
            transition.next = tokenizer_state_t::Date;
            transition.action = tokenizer_action_t::Capture;
        } else {
            transition.next = tokenizer_state_t::DashAfterInteger;
            transition.action = tokenizer_action_t::Found;
            transition.name = tokenizer_name_t::IntegerLiteral;
        }

        r[i] = transition;
    }
    return r;
}

constexpr std::array<tokenizer_transition_t, 256> calculateTransitionTable_ColonAfterNumber()
{
    std::array<tokenizer_transition_t, 256> r{};

    for (uint16_t i = 0; i < r.size(); i++) {
        hilet c = char_cast<char>(i);
        tokenizer_transition_t transition = {':'};

        if (is_digit(c)) {
            transition.next = tokenizer_state_t::Time;
            transition.action = tokenizer_action_t::Capture;
        } else {
            transition.next = tokenizer_state_t::ColonAfterInteger;
            transition.action = tokenizer_action_t::Found;
            transition.name = tokenizer_name_t::IntegerLiteral;
        }

        r[i] = transition;
    }
    return r;
}

constexpr std::array<tokenizer_transition_t, 256> calculateTransitionTable_DashAfterInteger()
{
    std::array<tokenizer_transition_t, 256> r{};

    for (uint16_t i = 0; i < r.size(); i++) {
        tokenizer_transition_t transition = {'-'};
        transition.next = tokenizer_state_t::OperatorSecondChar;
        transition.action = tokenizer_action_t::Start | tokenizer_action_t::Capture;

        r[i] = transition;
    }
    return r;
}

constexpr std::array<tokenizer_transition_t, 256> calculateTransitionTable_ColonAfterInteger()
{
    std::array<tokenizer_transition_t, 256> r{};

    for (uint16_t i = 0; i < r.size(); i++) {
        tokenizer_transition_t transition = {':'};
        transition.next = tokenizer_state_t::OperatorSecondChar;
        transition.action = tokenizer_action_t::Start | tokenizer_action_t::Capture;

        r[i] = transition;
    }
    return r;
}

constexpr std::array<tokenizer_transition_t, 256> calculateTransitionTable_Date()
{
    std::array<tokenizer_transition_t, 256> r{};

    for (uint16_t i = 0; i < r.size(); i++) {
        hilet c = char_cast<char>(i);
        tokenizer_transition_t transition = {c};

        if (is_digit(c) || c == '-') {
            transition.next = tokenizer_state_t::Date;
            transition.action = tokenizer_action_t::Read | tokenizer_action_t::Capture;
        } else {
            transition.next = tokenizer_state_t::Initial;
            transition.action = tokenizer_action_t::Found;
            transition.name = tokenizer_name_t::DateLiteral;
        }

        r[i] = transition;
    }
    return r;
}

constexpr std::array<tokenizer_transition_t, 256> calculateTransitionTable_Time()
{
    std::array<tokenizer_transition_t, 256> r{};

    for (uint16_t i = 0; i < r.size(); i++) {
        hilet c = char_cast<char>(i);
        tokenizer_transition_t transition = {c};

        if (is_digit(c) || c == ':' || c == '.') {
            transition.next = tokenizer_state_t::Time;
            transition.action = tokenizer_action_t::Read | tokenizer_action_t::Capture;
        } else {
            transition.next = tokenizer_state_t::Initial;
            transition.action = tokenizer_action_t::Found;
            transition.name = tokenizer_name_t::TimeLiteral;
        }

        r[i] = transition;
    }
    return r;
}
constexpr std::array<tokenizer_transition_t, 256> calculateTransitionTable_Float()
{
    std::array<tokenizer_transition_t, 256> r{};

    for (uint16_t i = 0; i < r.size(); i++) {
        hilet c = char_cast<char>(i);
        tokenizer_transition_t transition = {c};

        if (is_digit(c) || c == 'e' || c == 'E' || c == '-') {
            transition.next = tokenizer_state_t::Float;
            transition.action = tokenizer_action_t::Read | tokenizer_action_t::Capture;
        } else if (c == '_' || c == '\'') {
            transition.next = tokenizer_state_t::Float;
            transition.action = tokenizer_action_t::Read;
        } else {
            transition.next = tokenizer_state_t::Initial;
            transition.action = tokenizer_action_t::Found;
            transition.name = tokenizer_name_t::FloatLiteral;
        }

        r[i] = transition;
    }
    return r;
}

constexpr std::array<tokenizer_transition_t, 256> calculateTransitionTable_Slash()
{
    std::array<tokenizer_transition_t, 256> r{};

    for (uint16_t i = 0; i < r.size(); i++) {
        hilet c = char_cast<char>(i);
        tokenizer_transition_t transition = {c};

        if (c == '/') {
            transition.next = tokenizer_state_t::LineComment;
            transition.action = tokenizer_action_t::Read | tokenizer_action_t::Start;
        } else if (c == '*') {
            transition.next = tokenizer_state_t::BlockComment;
            transition.action = tokenizer_action_t::Read | tokenizer_action_t::Start;
        } else {
            transition.next = tokenizer_state_t::OperatorSecondChar;
        }

        r[i] = transition;
    }
    return r;
}

constexpr std::array<tokenizer_transition_t, 256> calculateTransitionTable_LineComment()
{
    std::array<tokenizer_transition_t, 256> r{};

    for (uint16_t i = 0; i < r.size(); i++) {
        hilet c = char_cast<char>(i);
        tokenizer_transition_t transition = {c};

        if (c == '\0') {
            transition.next = tokenizer_state_t::Initial;
        } else if (is_line_feed(c)) {
            transition.next = tokenizer_state_t::Initial;
            transition.action = tokenizer_action_t::Read | c;
        } else {
            transition.next = tokenizer_state_t::LineComment;
            transition.action = tokenizer_action_t::Read | c;
        }

        r[i] = transition;
    }
    return r;
}

constexpr std::array<tokenizer_transition_t, 256> calculateTransitionTable_BlockComment()
{
    std::array<tokenizer_transition_t, 256> r{};

    for (uint16_t i = 0; i < r.size(); i++) {
        hilet c = char_cast<char>(i);
        tokenizer_transition_t transition = {c};

        if (c == '\0') {
            transition.next = tokenizer_state_t::Initial;
            transition.action = tokenizer_action_t::Found;
            transition.name = tokenizer_name_t::ErrorEOTInBlockComment;
        } else if (c == '*') {
            transition.next = tokenizer_state_t::BlockCommentMaybeEnd;
            transition.action = tokenizer_action_t::Read;
        } else {
            transition.next = tokenizer_state_t::BlockComment;
            transition.action = tokenizer_action_t::Read | c;
        }

        r[i] = transition;
    }
    return r;
}

constexpr std::array<tokenizer_transition_t, 256> calculateTransitionTable_BlockCommentMaybeEnd()
{
    std::array<tokenizer_transition_t, 256> r{};

    for (uint16_t i = 0; i < r.size(); i++) {
        hilet c = char_cast<char>(i);
        tokenizer_transition_t transition = {c};

        if (c == '\0') {
            transition.next = tokenizer_state_t::Initial;
            transition.action = tokenizer_action_t::Found;
            transition.name = tokenizer_name_t::ErrorEOTInBlockComment;
        } else if (c == '/') {
            transition.next = tokenizer_state_t::Initial;
            transition.action = tokenizer_action_t::Read;
        } else if (c == '*') {
            transition.next = tokenizer_state_t::BlockCommentMaybeEnd;
            transition.action = tokenizer_action_t::Read;
        } else {
            transition.next = tokenizer_state_t::BlockComment;
            transition.action = tokenizer_action_t::Read | c;
        }

        r[i] = transition;
    }
    return r;
}

constexpr std::array<tokenizer_transition_t, 256> calculateTransitionTable_Quote()
{
    std::array<tokenizer_transition_t, 256> r{};

    for (uint16_t i = 0; i < r.size(); i++) {
        hilet c = char_cast<char>(i);
        tokenizer_transition_t transition = {c};

        if (c == '\'') {
            // Empty string.
            transition.next = tokenizer_state_t::Initial;
            transition.action = tokenizer_action_t::Read | tokenizer_action_t::Found;
            transition.name = tokenizer_name_t::StringLiteral;

        } else {
            transition.next = tokenizer_state_t::QuoteString;
        }

        r[i] = transition;
    }
    return r;
}

constexpr std::array<tokenizer_transition_t, 256> calculateTransitionTable_DQuote()
{
    std::array<tokenizer_transition_t, 256> r{};

    for (uint16_t i = 0; i < r.size(); i++) {
        hilet c = char_cast<char>(i);
        tokenizer_transition_t transition = {c};

        if (c == '"') {
            transition.next = tokenizer_state_t::DoubleDQuote;
            transition.action = tokenizer_action_t::Read;
        } else {
            transition.next = tokenizer_state_t::DQuoteString;
        }

        r[i] = transition;
    }
    return r;
}

constexpr std::array<tokenizer_transition_t, 256> calculateTransitionTable_DoubleDQuote()
{
    std::array<tokenizer_transition_t, 256> r{};

    for (uint16_t i = 0; i < r.size(); i++) {
        hilet c = char_cast<char>(i);
        tokenizer_transition_t transition = {c};

        if (c == '"') {
            transition.next = tokenizer_state_t::BlockString;
            transition.action = tokenizer_action_t::Read;
        } else {
            // Empty string.
            transition.next = tokenizer_state_t::Initial;
            transition.action = tokenizer_action_t::Found;
            transition.name = tokenizer_name_t::StringLiteral;
        }

        r[i] = transition;
    }
    return r;
}

constexpr std::array<tokenizer_transition_t, 256> calculateTransitionTable_QuoteString()
{
    std::array<tokenizer_transition_t, 256> r{};

    for (uint16_t i = 0; i < r.size(); i++) {
        hilet c = char_cast<char>(i);
        tokenizer_transition_t transition = {c};

        if (c == '\0') {
            transition.next = tokenizer_state_t::Initial;
            transition.action = tokenizer_action_t::Found;
            transition.name = tokenizer_name_t::ErrorEOTInString;
        } else if (is_line_feed(c)) {
            transition.next = tokenizer_state_t::Initial;
            transition.action = tokenizer_action_t::Found | tokenizer_action_t::Read | tokenizer_action_t::Capture |
                tokenizer_action_t::Start | c;
            transition.name = tokenizer_name_t::ErrorLFInString;
        } else if (c == '\\') {
            transition.next = tokenizer_state_t::QuoteStringEscape;
            transition.action = tokenizer_action_t::Read;
        } else if (c == '\'') {
            transition.next = tokenizer_state_t::Initial;
            transition.action = tokenizer_action_t::Found | tokenizer_action_t::Read;
            transition.name = tokenizer_name_t::StringLiteral;
        } else {
            transition.next = tokenizer_state_t::QuoteString;
            transition.action = tokenizer_action_t::Read | tokenizer_action_t::Capture | c;
        }

        r[i] = transition;
    }
    return r;
}

constexpr std::array<tokenizer_transition_t, 256> calculateTransitionTable_DQuoteString()
{
    std::array<tokenizer_transition_t, 256> r{};

    for (uint16_t i = 0; i < r.size(); i++) {
        hilet c = char_cast<char>(i);
        tokenizer_transition_t transition = {c};

        if (c == '\0') {
            transition.next = tokenizer_state_t::Initial;
            transition.action = tokenizer_action_t::Found;
            transition.name = tokenizer_name_t::ErrorEOTInString;
        } else if (is_line_feed(c)) {
            transition.next = tokenizer_state_t::Initial;
            transition.action = tokenizer_action_t::Found | tokenizer_action_t::Read | tokenizer_action_t::Capture |
                tokenizer_action_t::Start | c;
            transition.name = tokenizer_name_t::ErrorLFInString;
        } else if (c == '\\') {
            transition.next = tokenizer_state_t::DQuoteStringEscape;
            transition.action = tokenizer_action_t::Read;
        } else if (c == '"') {
            transition.next = tokenizer_state_t::Initial;
            transition.action = tokenizer_action_t::Found | tokenizer_action_t::Read;
            transition.name = tokenizer_name_t::StringLiteral;
        } else {
            transition.next = tokenizer_state_t::DQuoteString;
            transition.action = tokenizer_action_t::Read | tokenizer_action_t::Capture | c;
        }

        r[i] = transition;
    }
    return r;
}

constexpr std::array<tokenizer_transition_t, 256> calculateTransitionTable_QuoteStringEscape()
{
    std::array<tokenizer_transition_t, 256> r{};

    for (uint16_t i = 0; i < r.size(); i++) {
        hilet c = char_cast<char>(i);
        tokenizer_transition_t transition = {c};

        switch (c) {
        case '\0':
            transition.next = tokenizer_state_t::Initial;
            transition.action = tokenizer_action_t::Found;
            transition.name = tokenizer_name_t::ErrorEOTInString;
            r[i] = transition;
            continue;

        case 'a': transition.c = '\a'; break;
        case 'b': transition.c = '\b'; break;
        case 'f': transition.c = '\f'; break;
        case 'n': transition.c = '\n'; break;
        case 'r': transition.c = '\r'; break;
        case 't': transition.c = '\t'; break;
        case 'v': transition.c = '\v'; break;
        }

        transition.next = tokenizer_state_t::QuoteString;
        transition.action = tokenizer_action_t::Read | tokenizer_action_t::Capture;
        r[i] = transition;
    }
    return r;
}

constexpr std::array<tokenizer_transition_t, 256> calculateTransitionTable_DQuoteStringEscape()
{
    std::array<tokenizer_transition_t, 256> r{};

    for (uint16_t i = 0; i < r.size(); i++) {
        hilet c = char_cast<char>(i);
        tokenizer_transition_t transition = {c};

        switch (c) {
        case '\0':
            transition.next = tokenizer_state_t::Initial;
            transition.action = tokenizer_action_t::Found;
            transition.name = tokenizer_name_t::ErrorEOTInString;
            r[i] = transition;
            continue;

        case 'a': transition.c = '\a'; break;
        case 'b': transition.c = '\b'; break;
        case 'f': transition.c = '\f'; break;
        case 'n': transition.c = '\n'; break;
        case 'r': transition.c = '\r'; break;
        case 't': transition.c = '\t'; break;
        case 'v': transition.c = '\v'; break;
        }

        transition.next = tokenizer_state_t::DQuoteString;
        transition.action = tokenizer_action_t::Read | tokenizer_action_t::Capture;
        r[i] = transition;
    }
    return r;
}

constexpr std::array<tokenizer_transition_t, 256> calculateTransitionTable_BlockString()
{
    std::array<tokenizer_transition_t, 256> r{};

    for (uint16_t i = 0; i < r.size(); i++) {
        hilet c = char_cast<char>(i);
        tokenizer_transition_t transition = {c};

        if (c == '\0') {
            transition.next = tokenizer_state_t::Initial;
            transition.action = tokenizer_action_t::Found;
            transition.name = tokenizer_name_t::ErrorEOTInString;
        } else if (c == '"') {
            transition.next = tokenizer_state_t::BlockStringDQuote;
            transition.action = tokenizer_action_t::Read;
        } else if (is_white_space(c)) {
            transition.next = tokenizer_state_t::BlockString;
            transition.action = tokenizer_action_t::Read | tokenizer_action_t::Capture | c;
        } else if (c == '\\') {
            transition.next = tokenizer_state_t::BlockStringEscape;
            transition.action = tokenizer_action_t::Read;
        } else {
            transition.next = tokenizer_state_t::BlockString;
            transition.action = tokenizer_action_t::Read | tokenizer_action_t::Capture;
        }

        r[i] = transition;
    }
    return r;
}

constexpr std::array<tokenizer_transition_t, 256> calculateTransitionTable_BlockStringDQuote()
{
    std::array<tokenizer_transition_t, 256> r{};

    for (uint16_t i = 0; i < r.size(); i++) {
        hilet c = char_cast<char>(i);
        tokenizer_transition_t transition = {c};

        if (c == '"') {
            transition.next = tokenizer_state_t::BlockStringDoubleDQuote;
            transition.action = tokenizer_action_t::Read;
        } else {
            transition.next = tokenizer_state_t::BlockString;
            transition.action = tokenizer_action_t::Capture;
            transition.c = '"';
        }

        r[i] = transition;
    }
    return r;
}

constexpr std::array<tokenizer_transition_t, 256> calculateTransitionTable_BlockStringDoubleDQuote()
{
    std::array<tokenizer_transition_t, 256> r{};

    for (uint16_t i = 0; i < r.size(); i++) {
        hilet c = char_cast<char>(i);
        tokenizer_transition_t transition = {c};

        if (c == '"') {
            transition.next = tokenizer_state_t::Initial;
            transition.action = tokenizer_action_t::Found | tokenizer_action_t::Read;
            transition.name = tokenizer_name_t::StringLiteral;
        } else {
            transition.next = tokenizer_state_t::BlockStringCaptureDQuote;
            transition.action = tokenizer_action_t::Capture;
            transition.c = '"';
        }

        r[i] = transition;
    }
    return r;
}

constexpr std::array<tokenizer_transition_t, 256> calculateTransitionTable_BlockStringCaptureDQuote()
{
    std::array<tokenizer_transition_t, 256> r{};

    for (uint16_t i = 0; i < r.size(); i++) {
        hilet c = char_cast<char>(i);
        tokenizer_transition_t transition = {c};

        transition.next = tokenizer_state_t::BlockString;
        transition.action = tokenizer_action_t::Capture;
        transition.c = '"';

        r[i] = transition;
    }
    return r;
}

constexpr std::array<tokenizer_transition_t, 256> calculateTransitionTable_BlockStringEscape()
{
    std::array<tokenizer_transition_t, 256> r{};

    for (uint16_t i = 0; i < r.size(); i++) {
        hilet c = char_cast<char>(i);
        tokenizer_transition_t transition = {c};

        switch (c) {
        case '\0':
            transition.next = tokenizer_state_t::Initial;
            transition.action = tokenizer_action_t::Found;
            transition.name = tokenizer_name_t::ErrorEOTInString;
            r[i] = transition;
            continue;

        case 'a': transition.c = '\a'; break;
        case 'b': transition.c = '\b'; break;
        case 'f': transition.c = '\f'; break;
        case 'n': transition.c = '\n'; break;
        case 'r': transition.c = '\r'; break;
        case 't': transition.c = '\t'; break;
        case 'v': transition.c = '\v'; break;
        }

        transition.next = tokenizer_state_t::BlockString;
        transition.action = tokenizer_action_t::Read | tokenizer_action_t::Capture;
        r[i] = transition;
    }
    return r;
}

constexpr std::array<tokenizer_transition_t, 256> calculateTransitionTable_OperatorThirdChar()
{
    std::array<tokenizer_transition_t, 256> r{};

    for (uint16_t i = 0; i < r.size(); i++) {
        hilet c = char_cast<char>(i);
        tokenizer_transition_t transition = {c};

        switch (c) {
        case '>':
        case '=':
            transition.next = tokenizer_state_t::Initial;
            transition.action = tokenizer_action_t::Found | tokenizer_action_t::Read | tokenizer_action_t::Capture;
            transition.name = tokenizer_name_t::Operator;
            break;
        default:
            transition.next = tokenizer_state_t::Initial;
            transition.action = tokenizer_action_t::Found;
            transition.name = tokenizer_name_t::Operator;
        }

        r[i] = transition;
    }
    return r;
}

constexpr std::array<tokenizer_transition_t, 256> calculateTransitionTable_OperatorSecondChar()
{
#define LAST_CHAR \
    transition.next = tokenizer_state_t::Initial; \
    transition.action = tokenizer_action_t::Found | tokenizer_action_t::Read | tokenizer_action_t::Capture; \
    transition.name = tokenizer_name_t::Operator

#define MORE_CHARS \
    transition.next = tokenizer_state_t::OperatorThirdChar; \
    transition.action = tokenizer_action_t::Read | tokenizer_action_t::Capture;

    std::array<tokenizer_transition_t, 256> r{};

    for (uint16_t i = 0; i < r.size(); i++) {
        hilet c = char_cast<char>(i);
        tokenizer_transition_t transition = {c};

        switch (c) {
        case '=': MORE_CHARS; break; // Possible: <=>
        case '<': MORE_CHARS; break; // Possible: <<=
        case '>': MORE_CHARS; break; // Possible: >>=

        case '-': LAST_CHAR; break;
        case '+': LAST_CHAR; break;
        case '*': LAST_CHAR; break;
        case '&': LAST_CHAR; break;
        case '|': LAST_CHAR; break;
        case '^': LAST_CHAR; break;
        default:
            transition.next = tokenizer_state_t::Initial;
            transition.action = tokenizer_action_t::Found;
            transition.name = tokenizer_name_t::Operator;
        }

        r[i] = transition;
    }
    return r;
#undef LAST_CHAR
#undef MORE_CHARS
}

constexpr std::array<tokenizer_transition_t, 256> calculateTransitionTable_ColonOperatorSecondChar()
{
#define LAST_CHAR \
    transition.next = tokenizer_state_t::Initial; \
    transition.action = tokenizer_action_t::Found | tokenizer_action_t::Read | tokenizer_action_t::Capture; \
    transition.name = tokenizer_name_t::Operator

    std::array<tokenizer_transition_t, 256> r{};

    for (uint16_t i = 0; i < r.size(); i++) {
        hilet c = char_cast<char>(i);
        tokenizer_transition_t transition = {c};

        switch (c) {
        case '=': LAST_CHAR; break;
        default:
            transition.next = tokenizer_state_t::Initial;
            transition.action = tokenizer_action_t::Found;
            transition.name = tokenizer_name_t::Operator;
        }

        r[i] = transition;
    }
    return r;
#undef LAST_CHAR
}

constexpr std::array<tokenizer_transition_t, 256> calculateTransitionTable_OperatorFirstChar()
{
#define LAST_CHAR \
    transition.next = tokenizer_state_t::Initial; \
    transition.action = tokenizer_action_t::Found | tokenizer_action_t::Read | tokenizer_action_t::Capture; \
    transition.name = tokenizer_name_t::Operator

#define MORE_CHARS \
    transition.next = tokenizer_state_t::OperatorSecondChar; \
    transition.action = tokenizer_action_t::Read | tokenizer_action_t::Capture;

    std::array<tokenizer_transition_t, 256> r{};

    for (uint16_t i = 0; i < r.size(); i++) {
        hilet c = char_cast<char>(i);
        tokenizer_transition_t transition = {c};

        switch (c) {
        case '.': LAST_CHAR; break;
        case ';': LAST_CHAR; break;
        case ',': LAST_CHAR; break;
        case '(': LAST_CHAR; break;
        case ')': LAST_CHAR; break;
        case '[': LAST_CHAR; break;
        case ']': LAST_CHAR; break;
        case '{': LAST_CHAR; break;
        case '}': LAST_CHAR; break;
        case '?': LAST_CHAR; break;
        case '@': LAST_CHAR; break;
        case '$': LAST_CHAR; break;
        case '~': LAST_CHAR; break;

        case '!': MORE_CHARS; break; // Possible: !=
        case '<': MORE_CHARS; break; // Possible: <=>, <=, <-, <<, <>, <<=
        case '>': MORE_CHARS; break; // Possible: >=, >>, >>=
        case '=': MORE_CHARS; break; // Possible: ==, =>
        case '+': MORE_CHARS; break; // Possible: ++, +=
        case '-': MORE_CHARS; break; // Possible: --, ->, -=
        case '*': MORE_CHARS; break; // Possible: **
        case '%': MORE_CHARS; break; // Possible: %=
        case '/': MORE_CHARS; break; // Possible: /=
        case '|': MORE_CHARS; break; // Possible: ||, |=
        case '&': MORE_CHARS; break; // Possible: &&, &=
        case '^': MORE_CHARS; break; // Possible: ^=
        case ':':
            transition.next = tokenizer_state_t::ColonOperatorSecondChar;
            transition.action = tokenizer_action_t::Read | tokenizer_action_t::Capture;
            break; // Possible: :=
        default:
            // If we don't recognize the operator, it means this character is invalid.
            transition.next = tokenizer_state_t::Initial;
            transition.action = tokenizer_action_t::Read | tokenizer_action_t::Capture;
            transition.name = tokenizer_name_t::ErrorInvalidCharacter;
        }

        r[i] = transition;
    }
    return r;
#undef LAST_CHAR
#undef MORE_CHARS
}

constexpr std::array<tokenizer_transition_t, 256> calculateTransitionTable_Initial()
{
    std::array<tokenizer_transition_t, 256> r{};

    for (uint16_t i = 0; i < r.size(); i++) {
        hilet c = char_cast<char>(i);
        tokenizer_transition_t transition = {c};

        if (c == '\0') {
            transition.next = tokenizer_state_t::Initial;
            transition.action = tokenizer_action_t::Found;
            transition.name = tokenizer_name_t::End;
        } else if (is_name_first(c)) {
            transition.next = tokenizer_state_t::Name;
            transition.action = tokenizer_action_t::Read | tokenizer_action_t::Capture | tokenizer_action_t::Start;
        } else if (c == '-' || c == '+') {
            transition.next = tokenizer_state_t::MinusOrPlus;
            transition.action = tokenizer_action_t::Read | tokenizer_action_t::Capture | tokenizer_action_t::Start;
        } else if (c == '0') {
            transition.next = tokenizer_state_t::Zero;
            transition.action = tokenizer_action_t::Read | tokenizer_action_t::Capture | tokenizer_action_t::Start;
        } else if (is_digit(c)) {
            transition.next = tokenizer_state_t::Number;
            transition.action = tokenizer_action_t::Read | tokenizer_action_t::Capture | tokenizer_action_t::Start;
        } else if (c == '.') {
            transition.next = tokenizer_state_t::Dot;
            transition.action = tokenizer_action_t::Read | tokenizer_action_t::Capture | tokenizer_action_t::Start;
        } else if (c == '"') {
            transition.next = tokenizer_state_t::DQuote;
            transition.action = tokenizer_action_t::Read | tokenizer_action_t::Start;
        } else if (c == '\'') {
            transition.next = tokenizer_state_t::Quote;
            transition.action = tokenizer_action_t::Read | tokenizer_action_t::Start;
        } else if (is_white_space(c)) {
            transition.next = tokenizer_state_t::Initial;
            transition.action = tokenizer_action_t::Read | c;
        } else if (c == '#') {
            transition.next = tokenizer_state_t::LineComment;
            transition.action = tokenizer_action_t::Read;
        } else if (c == '/') {
            transition.next = tokenizer_state_t::Slash;
            transition.action = tokenizer_action_t::Read | tokenizer_action_t::Capture | tokenizer_action_t::Start;
        } else {
            transition.next = tokenizer_state_t::OperatorFirstChar;
        }

        r[i] = transition;
    }
    return r;
}

constexpr std::size_t TRANSITION_TABLE_SIZE = NR_TOKENIZER_STATES * 256;
using transitionTable_t = std::array<tokenizer_transition_t, TRANSITION_TABLE_SIZE>;

constexpr transitionTable_t calculateTransitionTable()
{
#define CALCULATE_SUB_TABLE(x) \
    i = get_offset(tokenizer_state_t::x); \
    for (hilet t : calculateTransitionTable_##x()) { \
        r[i++] = t; \
    }

    transitionTable_t r{};

    // Poisson the table, to make sure all sub tables have been initialized.
    for (std::size_t i = 0; i < r.size(); i++) {
        r[i].action = tokenizer_action_t::Poison;
    }

    std::size_t i = 0;
    CALCULATE_SUB_TABLE(Initial);
    CALCULATE_SUB_TABLE(Name);
    CALCULATE_SUB_TABLE(MinusOrPlus);
    CALCULATE_SUB_TABLE(Zero);
    CALCULATE_SUB_TABLE(Dot);
    CALCULATE_SUB_TABLE(Number);
    CALCULATE_SUB_TABLE(DashAfterNumber);
    CALCULATE_SUB_TABLE(ColonAfterNumber);
    CALCULATE_SUB_TABLE(DashAfterInteger);
    CALCULATE_SUB_TABLE(ColonAfterInteger);
    CALCULATE_SUB_TABLE(Date);
    CALCULATE_SUB_TABLE(Time);
    CALCULATE_SUB_TABLE(Float);
    CALCULATE_SUB_TABLE(Quote);
    CALCULATE_SUB_TABLE(QuoteString);
    CALCULATE_SUB_TABLE(QuoteStringEscape);
    CALCULATE_SUB_TABLE(DQuote);
    CALCULATE_SUB_TABLE(DoubleDQuote);
    CALCULATE_SUB_TABLE(DQuoteString);
    CALCULATE_SUB_TABLE(DQuoteStringEscape);
    CALCULATE_SUB_TABLE(BlockString);
    CALCULATE_SUB_TABLE(BlockStringDQuote);
    CALCULATE_SUB_TABLE(BlockStringDoubleDQuote);
    CALCULATE_SUB_TABLE(BlockStringCaptureDQuote);
    CALCULATE_SUB_TABLE(BlockStringEscape);
    CALCULATE_SUB_TABLE(Slash);
    CALCULATE_SUB_TABLE(LineComment);
    CALCULATE_SUB_TABLE(BlockComment);
    CALCULATE_SUB_TABLE(BlockCommentMaybeEnd);
    CALCULATE_SUB_TABLE(OperatorFirstChar);
    CALCULATE_SUB_TABLE(OperatorSecondChar);
    CALCULATE_SUB_TABLE(OperatorThirdChar);
    CALCULATE_SUB_TABLE(ColonOperatorSecondChar);
    return r;
#undef CALCULATE_SUB_TABLE
}

constexpr bool optimizeTransitionTableOnce(transitionTable_t &r)
{
    bool foundOptimization = false;
    for (std::size_t i = 0; i < r.size(); i++) {
        auto &transition = r[i];
        if (transition.action == tokenizer_action_t::Idle) {
            foundOptimization = true;

            transition = r[get_offset(transition.next, char_cast<char>(i & 0xff))];
        }
    }
    return foundOptimization;
}

constexpr transitionTable_t optimizeTransitionTable(transitionTable_t transitionTable)
{
    for (int i = 0; i < 10; i++) {
        if (!optimizeTransitionTableOnce(transitionTable)) {
            break;
        }
    }
    return transitionTable;
}

constexpr bool checkTransitionTable(transitionTable_t const &r)
{
    for (std::size_t i = 0; i < r.size(); i++) {
        if (r[i].action >= tokenizer_action_t::Poison) {
            return false;
        }
    }
    return true;
}

constexpr transitionTable_t buildTransitionTable()
{
    constexpr auto transitionTable = calculateTransitionTable();
    static_assert(checkTransitionTable(transitionTable), "Not all entries in transition table where assigned.");
    return optimizeTransitionTable(transitionTable);
}

constexpr transitionTable_t transitionTable = buildTransitionTable();

struct tokenizer {
    using iterator = typename std::string_view::const_iterator;

    tokenizer_state_t state;
    iterator index;
    iterator end;
    parse_location location;
    parse_location captureLocation;

    tokenizer(iterator begin, iterator end) : state(tokenizer_state_t::Initial), index(begin), end(end) {}

    /*! Parse a token.
     */
    [[nodiscard]] token_t getNextToken()
    {
        auto token = token_t{};

        auto transition = tokenizer_transition_t{};
        while (index != end) {
            transition = transitionTable[get_offset(state, *index)];
            state = transition.next;

            auto action = transition.action;
            if (action >= tokenizer_action_t::Start) {
                token.location = location;
                token.value.clear();
            }

            if (action >= tokenizer_action_t::Capture) {
                token.value += transition.c;
            }

            if (action >= tokenizer_action_t::Read) {
                if (action >= tokenizer_action_t::LineFeed) {
                    location.increment_line();
                } else if (action >= tokenizer_action_t::Tab) {
                    location.tab_column();
                } else {
                    location.increment_column();
                }
                ++index;
            }

            if (action >= tokenizer_action_t::Found) {
                token.name = transition.name;
                return token;
            }
        }

        // Complete the token at the current state. Or an end-token at the initial state.
        if (state == tokenizer_state_t::Initial) {
            // Mark the current offset as the position of the end-token.
            token.location = location;
            token.value.clear();
        }

        transition = transitionTable[get_offset(state)];
        state = transition.next;

        token.name = transition.name;
        return token;
    }

    /*! Parse all tokens.
     */
    [[nodiscard]] std::vector<token_t> getTokens() noexcept
    {
        std::vector<token_t> r;

        tokenizer_name_t token_name;
        do {
            hilet token = getNextToken();
            token_name = token.name;
            r.push_back(std::move(token));
        } while (token_name != tokenizer_name_t::End);

        return r;
    }
};

[[nodiscard]] std::vector<token_t>
parseTokens(std::string_view::const_iterator first, std::string_view::const_iterator last) noexcept
{
    return tokenizer(first, last).getTokens();
}

[[nodiscard]] std::vector<token_t> parseTokens(std::string_view text) noexcept
{
    return parseTokens(text.cbegin(), text.cend());
}

} // namespace hi::inline v1
