// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Foundation/tokenizer.hpp"

namespace TTauri {

enum class tokenizer_state_t: uint8_t {
    Initial,
    Name,
    MinusOrPlus,            // Could be the start of a number, or an operator starting with '+' or '-'.
    Zero,                   // Could be part of a number with a base.
    Dot,                    // Could be the start of a floating point number, or the '.' operator.
    Number,                 // Could be some kind of number without a base.
    DashAfterNumber,        // Could be a date.
    ColonAfterNumber,       // Could be a time.
    DashAfterInteger,       // Integer followed by a operator starting with '-'
    ColonAfterInteger,      // Integer followed by a operator starting with ':'
    Float,
    Date,
    Time,
    String,
    StringEscape,
    Slash,                  // Could be the start of a LineComment, BlockComment, or an operator.
    LineComment,
    BlockComment,
    BlockCommentMaybeEnd,   // Found a '*' possibly end of comment.
    OperatorFirstChar,
    OperatorSecondChar,
    OperatorThirdChar,

    Sentinal
};
constexpr size_t NR_TOKENIZER_STATES = static_cast<size_t>(tokenizer_state_t::Sentinal);

enum class tokenizer_action_t: uint8_t {
    Idle = 0x00,
    Capture = 0x01, // Capture this character.
    Start = 0x02, // Start the capture queue.
    Read = 0x04, // Read next character, before processing next state.
    Found = 0x08, // Token Found.
};

constexpr uint16_t get_offset(tokenizer_state_t state, char c = '\0') noexcept {
    return (static_cast<uint16_t>(state) << 8) | static_cast<uint8_t>(c);
}

constexpr tokenizer_action_t operator|(tokenizer_action_t lhs, tokenizer_action_t rhs)
{
    return static_cast<tokenizer_action_t>(static_cast<uint8_t>(lhs) | static_cast<uint8_t>(rhs));
}

constexpr bool operator>=(tokenizer_action_t lhs, tokenizer_action_t rhs)
{
    return (static_cast<uint8_t>(lhs) & static_cast<uint8_t>(rhs)) == static_cast<uint8_t>(rhs);
}

struct tokenizer_transition_t {
    uint16_t actionAndNextState;
    char c;
    tokenizer_name_t name;

    constexpr tokenizer_transition_t(
        char c,
        tokenizer_state_t next = tokenizer_state_t::Initial,
        tokenizer_action_t action = tokenizer_action_t::Idle,
        tokenizer_name_t name = tokenizer_name_t::NotAssigned
    ) :
        actionAndNextState(get_offset(next) | static_cast<uint8_t>(action)),
        c(c),
        name(name) {}

    constexpr tokenizer_transition_t() :
        actionAndNextState(0xffff), c('\0'), name(tokenizer_name_t::NotAssigned) {}

    constexpr tokenizer_state_t next() const noexcept {
        uint16_t stateInt = actionAndNextState >> 8;
        ttauri_axiom(stateInt < static_cast<uint16_t>(tokenizer_state_t::Sentinal));
        return static_cast<tokenizer_state_t>(stateInt);
    }
    constexpr tokenizer_action_t action() const noexcept {
        uint8_t actionInt = static_cast<uint8_t>(actionAndNextState);
        return static_cast<tokenizer_action_t>(actionInt);
    }

    constexpr void setNext(tokenizer_state_t state) noexcept {
        actionAndNextState &= 0x00ff;
        actionAndNextState |= get_offset(state);
    }

    constexpr void setAction(tokenizer_action_t action) noexcept {
        actionAndNextState &= 0xff00;
        actionAndNextState |= static_cast<uint16_t>(action);
    }
};

constexpr std::array<tokenizer_transition_t,256> calculateTransitionTable_Name()
{
    std::array<tokenizer_transition_t,256> r{};

    for (uint16_t i = 0; i < r.size(); i++) {
        let c = static_cast<char>(i);
        tokenizer_transition_t transition = {c};

        if (isNameNext(c) || c == '-') {
            transition.setNext(tokenizer_state_t::Name);
            transition.setAction(tokenizer_action_t::Read | tokenizer_action_t::Capture);
        } else {
            transition.setNext(tokenizer_state_t::Initial);
            transition.setAction(tokenizer_action_t::Found);
            transition.name = tokenizer_name_t::Name;
        }

        r[i] = transition;
    }
    return r;
}

constexpr std::array<tokenizer_transition_t,256> calculateTransitionTable_MinusOrPlus()
{
    std::array<tokenizer_transition_t,256> r{};

    for (uint16_t i = 0; i < r.size(); i++) {
        let c = static_cast<char>(i);
        tokenizer_transition_t transition = {c};

        if (c == '0') {
            transition.setNext(tokenizer_state_t::Zero);
            transition.setAction(tokenizer_action_t::Read | tokenizer_action_t::Capture);
        } else if (isDigit(c) || c == '.') {
            transition.setNext(tokenizer_state_t::Number);
        } else {
            transition.setNext(tokenizer_state_t::OperatorSecondChar);
        }

        r[i] = transition;
    }
    return r;
}

constexpr std::array<tokenizer_transition_t,256> calculateTransitionTable_Dot()
{
    std::array<tokenizer_transition_t,256> r{};

    for (uint16_t i = 0; i < r.size(); i++) {
        let c = static_cast<char>(i);
        tokenizer_transition_t transition = {c};

        if (isDigit(c)) {
            transition.setNext(tokenizer_state_t::Float);
        } else {
            transition.setNext(tokenizer_state_t::Initial);
            transition.setAction(tokenizer_action_t::Found);
            transition.name = tokenizer_name_t::Operator;
        }

        r[i] = transition;
    }
    return r;
}

constexpr std::array<tokenizer_transition_t,256> calculateTransitionTable_Zero()
{
    std::array<tokenizer_transition_t,256> r{};

    for (uint16_t i = 0; i < r.size(); i++) {
        let c = static_cast<char>(i);
        tokenizer_transition_t transition = {c};

        if (c == 'x' || c == 'X' || c == 'd' || c == 'D' || c == 'o' || c == 'O' || c == 'b' || c == 'B') {
            transition.setNext(tokenizer_state_t::Number);
            transition.setAction(tokenizer_action_t::Read | tokenizer_action_t::Capture);
        } else {
            transition.setNext(tokenizer_state_t::Number);
        }

        r[i] = transition;
    }
    return r;
}

constexpr std::array<tokenizer_transition_t,256> calculateTransitionTable_Number()
{
    std::array<tokenizer_transition_t,256> r{};

    for (uint16_t i = 0; i < r.size(); i++) {
        let c = static_cast<char>(i);
        tokenizer_transition_t transition = {c};

        if (isDigit(c) || c == '_' || c == '\'') {
            transition.setNext(tokenizer_state_t::Number);
            transition.setAction(tokenizer_action_t::Read | tokenizer_action_t::Capture);
        } else if (c == '.') {
            transition.setNext(tokenizer_state_t::Float);
            transition.setAction(tokenizer_action_t::Read | tokenizer_action_t::Capture);
        } else if (c == '-') {
            transition.setNext(tokenizer_state_t::DashAfterNumber);
            transition.setAction(tokenizer_action_t::Read);
        } else if (c == ':') {
            transition.setNext(tokenizer_state_t::ColonAfterNumber);
            transition.setAction(tokenizer_action_t::Read);
        } else {
            transition.setNext(tokenizer_state_t::Initial);
            transition.setAction(tokenizer_action_t::Found);
            transition.name = tokenizer_name_t::IntegerLiteral;
        }

        r[i] = transition;
    }
    return r;
}

constexpr std::array<tokenizer_transition_t,256> calculateTransitionTable_DashAfterNumber()
{
    std::array<tokenizer_transition_t,256> r{};

    for (uint16_t i = 0; i < r.size(); i++) {
        let c = static_cast<char>(i);
        tokenizer_transition_t transition = {'-'};

        if (isDigit(c)) {
            transition.setNext(tokenizer_state_t::Date);
            transition.setAction(tokenizer_action_t::Capture);
        } else {
            transition.setNext(tokenizer_state_t::DashAfterInteger);
            transition.setAction(tokenizer_action_t::Found);
            transition.name = tokenizer_name_t::IntegerLiteral;
        }

        r[i] = transition;
    }
    return r;
}

constexpr std::array<tokenizer_transition_t,256> calculateTransitionTable_ColonAfterNumber()
{
    std::array<tokenizer_transition_t,256> r{};

    for (uint16_t i = 0; i < r.size(); i++) {
        let c = static_cast<char>(i);
        tokenizer_transition_t transition = {':'};

        if (isDigit(c)) {
            transition.setNext(tokenizer_state_t::Time);
            transition.setAction(tokenizer_action_t::Capture);
        } else {
            transition.setNext(tokenizer_state_t::ColonAfterInteger);
            transition.setAction(tokenizer_action_t::Found);
            transition.name = tokenizer_name_t::IntegerLiteral;
        }

        r[i] = transition;
    }
    return r;
}

constexpr std::array<tokenizer_transition_t,256> calculateTransitionTable_DashAfterInteger()
{
    std::array<tokenizer_transition_t,256> r{};

    for (uint16_t i = 0; i < r.size(); i++) {
        tokenizer_transition_t transition = {'-'};
        transition.setNext(tokenizer_state_t::OperatorSecondChar);
        transition.setAction(tokenizer_action_t::Start | tokenizer_action_t::Capture);

        r[i] = transition;
    }
    return r;
}

constexpr std::array<tokenizer_transition_t,256> calculateTransitionTable_ColonAfterInteger()
{
    std::array<tokenizer_transition_t,256> r{};

    for (uint16_t i = 0; i < r.size(); i++) {
        tokenizer_transition_t transition = {':'};
        transition.setNext(tokenizer_state_t::OperatorSecondChar);
        transition.setAction(tokenizer_action_t::Start | tokenizer_action_t::Capture);

        r[i] = transition;
    }
    return r;
}

constexpr std::array<tokenizer_transition_t,256> calculateTransitionTable_Date()
{
    std::array<tokenizer_transition_t,256> r{};

    for (uint16_t i = 0; i < r.size(); i++) {
        let c = static_cast<char>(i);
        tokenizer_transition_t transition = {c};

        if (isDigit(c) || c == '-') {
            transition.setNext(tokenizer_state_t::Date);
            transition.setAction(tokenizer_action_t::Read | tokenizer_action_t::Capture);
        } else {
            transition.setNext(tokenizer_state_t::Initial);
            transition.setAction(tokenizer_action_t::Found);
            transition.name = tokenizer_name_t::DateLiteral;
        }

        r[i] = transition;
    }
    return r;
}

constexpr std::array<tokenizer_transition_t,256> calculateTransitionTable_Time()
{
    std::array<tokenizer_transition_t,256> r{};

    for (uint16_t i = 0; i < r.size(); i++) {
        let c = static_cast<char>(i);
        tokenizer_transition_t transition = {c};

        if (isDigit(c) || c == ':' || c == '.') {
            transition.setNext(tokenizer_state_t::Time);
            transition.setAction(tokenizer_action_t::Read | tokenizer_action_t::Capture);
        } else {
            transition.setNext(tokenizer_state_t::Initial);
            transition.setAction(tokenizer_action_t::Found);
            transition.name = tokenizer_name_t::TimeLiteral;
        }

        r[i] = transition;
    }
    return r;
}
constexpr std::array<tokenizer_transition_t,256> calculateTransitionTable_Float()
{
    std::array<tokenizer_transition_t,256> r{};

    for (uint16_t i = 0; i < r.size(); i++) {
        let c = static_cast<char>(i);
        tokenizer_transition_t transition = {c};

        if (isDigit(c) || c == 'e' || c == 'E' || c == '-') {
            transition.setNext(tokenizer_state_t::Float);
            transition.setAction(tokenizer_action_t::Read | tokenizer_action_t::Capture);
        } else if (c == '_' || c == '\'') {
            transition.setNext(tokenizer_state_t::Float);
            transition.setAction(tokenizer_action_t::Read);
        } else {
            transition.setNext(tokenizer_state_t::Initial);
            transition.setAction(tokenizer_action_t::Found);
            transition.name = tokenizer_name_t::FloatLiteral;
        }

        r[i] = transition;
    }
    return r;
}

constexpr std::array<tokenizer_transition_t,256> calculateTransitionTable_Slash()
{
    std::array<tokenizer_transition_t,256> r{};

    for (uint16_t i = 0; i < r.size(); i++) {
        let c = static_cast<char>(i);
        tokenizer_transition_t transition = {c};

        if (c == '/') {
            transition.setNext(tokenizer_state_t::LineComment);
            transition.setAction(tokenizer_action_t::Read);
        } else if (c == '*') {
            transition.setNext(tokenizer_state_t::BlockComment);
            transition.setAction(tokenizer_action_t::Read);
        } else {
            transition.setNext(tokenizer_state_t::OperatorSecondChar);
        }

        r[i] = transition;
    }
    return r;
}

constexpr std::array<tokenizer_transition_t,256> calculateTransitionTable_LineComment()
{
    std::array<tokenizer_transition_t,256> r{};

    for (uint16_t i = 0; i < r.size(); i++) {
        let c = static_cast<char>(i);
        tokenizer_transition_t transition = {c};

        if (c == '\0') {
            transition.setNext(tokenizer_state_t::Initial);
        } else if (isLinefeed(c)) {
            transition.setNext(tokenizer_state_t::Initial);
            transition.setAction(tokenizer_action_t::Read);
        } else {
            transition.setNext(tokenizer_state_t::LineComment);
            transition.setAction(tokenizer_action_t::Read);
        }

        r[i] = transition;
    }
    return r;
}

constexpr std::array<tokenizer_transition_t,256> calculateTransitionTable_BlockComment()
{
    std::array<tokenizer_transition_t,256> r{};

    for (uint16_t i = 0; i < r.size(); i++) {
        let c = static_cast<char>(i);
        tokenizer_transition_t transition = {c};

        if (c == '\0') {
            transition.setNext(tokenizer_state_t::Initial);
            transition.setAction(tokenizer_action_t::Found);
            transition.name = tokenizer_name_t::ErrorEOTInBlockComment;
        } else if (c == '*') {
            transition.setNext(tokenizer_state_t::BlockCommentMaybeEnd);
            transition.setAction(tokenizer_action_t::Read);
        } else {
            transition.setNext(tokenizer_state_t::BlockComment);
            transition.setAction(tokenizer_action_t::Read);
        }

        r[i] = transition;
    }
    return r;
}

constexpr std::array<tokenizer_transition_t,256> calculateTransitionTable_BlockCommentMaybeEnd()
{
    std::array<tokenizer_transition_t,256> r{};

    for (uint16_t i = 0; i < r.size(); i++) {
        let c = static_cast<char>(i);
        tokenizer_transition_t transition = {c};

        if (c == '\0') {
            transition.setNext(tokenizer_state_t::Initial);
            transition.setAction(tokenizer_action_t::Found);
            transition.name = tokenizer_name_t::ErrorEOTInBlockComment;
        } else if (c == '/') {
            transition.setNext(tokenizer_state_t::Initial);
            transition.setAction(tokenizer_action_t::Read);
        } else if (c == '*') {
            transition.setNext(tokenizer_state_t::BlockCommentMaybeEnd);
            transition.setAction(tokenizer_action_t::Read);
        } else {
            transition.setNext(tokenizer_state_t::BlockComment);
            transition.setAction(tokenizer_action_t::Read);
        }

        r[i] = transition;
    }
    return r;
}

constexpr std::array<tokenizer_transition_t,256> calculateTransitionTable_String()
{
    std::array<tokenizer_transition_t,256> r{};

    for (uint16_t i = 0; i < r.size(); i++) {
        let c = static_cast<char>(i);
        tokenizer_transition_t transition = {c};

        if (c == '\0') {
            transition.setNext(tokenizer_state_t::Initial);
            transition.setAction(tokenizer_action_t::Found);
            transition.name = tokenizer_name_t::ErrorEOTInString;
        } else if (isLinefeed(c)) {
            transition.setNext(tokenizer_state_t::Initial);
            transition.setAction(tokenizer_action_t::Found | tokenizer_action_t::Read | tokenizer_action_t::Capture | tokenizer_action_t::Start);
            transition.name = tokenizer_name_t::ErrorLFInString;
        } else if (c == '\\') {
            transition.setNext(tokenizer_state_t::StringEscape);
            transition.setAction(tokenizer_action_t::Read);
        } else if (c == '"') {
            transition.setNext(tokenizer_state_t::Initial);
            transition.setAction(tokenizer_action_t::Found | tokenizer_action_t::Read);
            transition.name = tokenizer_name_t::StringLiteral;
        } else {
            transition.setNext(tokenizer_state_t::String);
            transition.setAction(tokenizer_action_t::Read | tokenizer_action_t::Capture);
        }

        r[i] = transition;
    }
    return r;
}

constexpr std::array<tokenizer_transition_t,256> calculateTransitionTable_StringEscape()
{
    std::array<tokenizer_transition_t,256> r{};

    for (uint16_t i = 0; i < r.size(); i++) {
        let c = static_cast<char>(i);
        tokenizer_transition_t transition = {c};

        switch (c) {
        case '\0':
            transition.setNext(tokenizer_state_t::Initial);
            transition.setAction(tokenizer_action_t::Found);
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

        transition.setNext(tokenizer_state_t::String);
        transition.setAction(tokenizer_action_t::Read | tokenizer_action_t::Capture);
        r[i] = transition;
    }
    return r;
}

constexpr std::array<tokenizer_transition_t,256> calculateTransitionTable_OperatorThirdChar()
{
    std::array<tokenizer_transition_t,256> r{};

    for (uint16_t i = 0; i < r.size(); i++) {
        let c = static_cast<char>(i);
        tokenizer_transition_t transition = {c};

        switch (c) {
        case '>':
        case '=':
            transition.setNext(tokenizer_state_t::Initial);
            transition.setAction(tokenizer_action_t::Found | tokenizer_action_t::Read | tokenizer_action_t::Capture);
            transition.name = tokenizer_name_t::Operator;
            break;
        default:
            transition.setNext(tokenizer_state_t::Initial);
            transition.setAction(tokenizer_action_t::Found);
            transition.name = tokenizer_name_t::Operator;
        }

        r[i] = transition;
    }
    return r;
}

constexpr std::array<tokenizer_transition_t,256> calculateTransitionTable_OperatorSecondChar()
{
#define LAST_CHAR\
    transition.setNext(tokenizer_state_t::Initial);\
    transition.setAction(tokenizer_action_t::Found | tokenizer_action_t::Read | tokenizer_action_t::Capture);\
    transition.name = tokenizer_name_t::Operator

#define MORE_CHARS\
    transition.setNext(tokenizer_state_t::OperatorThirdChar);\
    transition.setAction(tokenizer_action_t::Read | tokenizer_action_t::Capture);

    std::array<tokenizer_transition_t,256> r{};

    for (uint16_t i = 0; i < r.size(); i++) {
        let c = static_cast<char>(i);
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
            transition.setNext(tokenizer_state_t::Initial);
            transition.setAction(tokenizer_action_t::Found);
            transition.name = tokenizer_name_t::Operator;
        }

        r[i] = transition;
    }
    return r;
#undef LAST_CHAR
#undef MORE_CHARS
}

constexpr std::array<tokenizer_transition_t,256> calculateTransitionTable_OperatorFirstChar()
{
#define LAST_CHAR\
    transition.setNext(tokenizer_state_t::Initial);\
    transition.setAction(tokenizer_action_t::Found | tokenizer_action_t::Read | tokenizer_action_t::Capture);\
    transition.name = tokenizer_name_t::Operator

#define MORE_CHARS\
    transition.setNext(tokenizer_state_t::OperatorSecondChar);\
    transition.setAction(tokenizer_action_t::Read | tokenizer_action_t::Capture);

    std::array<tokenizer_transition_t,256> r{};

    for (uint16_t i = 0; i < r.size(); i++) {
        let c = static_cast<char>(i);
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
        case ':': MORE_CHARS; break; // Possible: :=
        default:
            // If we don't recognize the operator, it means this character is invalid.
            transition.setNext(tokenizer_state_t::Initial);
            transition.setAction(tokenizer_action_t::Read | tokenizer_action_t::Capture);
            transition.name = tokenizer_name_t::ErrorInvalidCharacter;
        }

        r[i] = transition;
    }
    return r;
#undef LAST_CHAR
#undef MORE_CHARS
}

constexpr std::array<tokenizer_transition_t,256> calculateTransitionTable_Initial()
{
    std::array<tokenizer_transition_t,256> r{};

    for (uint16_t i = 0; i < r.size(); i++) {
        let c = static_cast<char>(i);
        tokenizer_transition_t transition = {c};

        if (c == '\0') {
            transition.setNext(tokenizer_state_t::Initial);
            transition.setAction(tokenizer_action_t::Found);
            transition.name = tokenizer_name_t::End;
        } else if (isNameFirst(c)) {
            transition.setNext(tokenizer_state_t::Name);
            transition.setAction(tokenizer_action_t::Read | tokenizer_action_t::Capture | tokenizer_action_t::Start);
        } else if (c == '-' || c == '+') {
            transition.setNext(tokenizer_state_t::MinusOrPlus);
            transition.setAction(tokenizer_action_t::Read | tokenizer_action_t::Capture | tokenizer_action_t::Start);
        } else if (c == '0') {
            transition.setNext(tokenizer_state_t::Zero);
            transition.setAction(tokenizer_action_t::Read | tokenizer_action_t::Capture | tokenizer_action_t::Start);
        } else if (isDigit(c)) {
            transition.setNext(tokenizer_state_t::Number);
            transition.setAction(tokenizer_action_t::Read | tokenizer_action_t::Capture | tokenizer_action_t::Start);
        } else if (c == '.') {
            transition.setNext(tokenizer_state_t::Dot);
            transition.setAction(tokenizer_action_t::Read | tokenizer_action_t::Capture | tokenizer_action_t::Start);
        } else if (c == '"') {
            transition.setNext(tokenizer_state_t::String);
            transition.setAction(tokenizer_action_t::Read | tokenizer_action_t::Start);
        } else if (isWhitespace(c)) {
            transition.setNext(tokenizer_state_t::Initial);
            transition.setAction(tokenizer_action_t::Read);
        } else if (c == '#') {
            transition.setNext(tokenizer_state_t::LineComment);
            transition.setAction(tokenizer_action_t::Read);
        } else if (c == '/') {
            transition.setNext(tokenizer_state_t::Slash);
            transition.setAction(tokenizer_action_t::Read | tokenizer_action_t::Capture | tokenizer_action_t::Start);
        } else {
            transition.setNext(tokenizer_state_t::OperatorFirstChar);
        }

        r[i] = transition;
    }
    return r;
}

constexpr size_t TRANSITION_TABLE_SIZE = NR_TOKENIZER_STATES * 256;
using transitionTable_t = std::array<tokenizer_transition_t,TRANSITION_TABLE_SIZE>;



constexpr transitionTable_t calculateTransitionTable()
{
#define CALCULATE_SUB_TABLE(x)\
    i = get_offset(tokenizer_state_t::x);\
    for (let t: calculateTransitionTable_ ## x()) { r[i++] = t; }

    transitionTable_t r{};

    // Poisson the table, to make sure all sub tables have been initialized.
    for (size_t i = 0; i < r.size(); i++) {
        r[i].actionAndNextState = 0xffff;
    }

    size_t i = 0;
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
    CALCULATE_SUB_TABLE(String);
    CALCULATE_SUB_TABLE(StringEscape);
    CALCULATE_SUB_TABLE(Slash);
    CALCULATE_SUB_TABLE(LineComment);
    CALCULATE_SUB_TABLE(BlockComment);
    CALCULATE_SUB_TABLE(BlockCommentMaybeEnd);
    CALCULATE_SUB_TABLE(OperatorFirstChar);
    CALCULATE_SUB_TABLE(OperatorSecondChar);
    CALCULATE_SUB_TABLE(OperatorThirdChar);

    return r;
#undef CALCULATE_SUB_TABLE
}

constexpr bool optimizeTransitionTableOnce(transitionTable_t &r)
{
    bool foundOptimization = false;
    for (size_t i = 0; i < r.size(); i++) {
        auto &transition = r[i];
        if (transition.action() == tokenizer_action_t::Idle) {
            foundOptimization = true;

            transition = r[get_offset(transition.next(), static_cast<char>(i & 0xff))];
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
    for (size_t i = 0; i < r.size(); i++) {
        if (r[i].actionAndNextState == 0xffff) {
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
    iterator begin;
    iterator end;
    iterator index;
    iterator captureIndex;

    tokenizer(iterator begin, iterator end) :
        state(tokenizer_state_t::Initial), begin(begin), end(end), index(begin), captureIndex(begin) {}

    template<uint8_t Action>
    static force_inline void executeAction(
        small_vector<char,256> &capture,
        iterator &captureIndex,
        iterator &index,
        tokenizer_state_t &state,
        tokenizer_transition_t &transition
    ) {
        constexpr auto action = static_cast<tokenizer_action_t>(Action);
        if constexpr (action >= tokenizer_action_t::Start) {
            captureIndex = index;
            capture.clear();
        }

        if constexpr (action >= tokenizer_action_t::Capture) {
            capture.push_back(transition.c);
        }

        if constexpr (action >= tokenizer_action_t::Read) {
            index++;
        }

        state = transition.next();
    }

    /*! Parse a token.
    */
    [[nodiscard]] token_t getNextToken() {
        small_vector<char,256> capture;

        auto _state = state;
        auto _index = index;
        auto transition = tokenizer_transition_t{};
        while (_index != end) {
            transition = transitionTable[get_offset(_state, *_index)];

            switch (static_cast<uint8_t>(transition.action())) {
            case 0x0: executeAction<0x0>(capture, captureIndex, _index, _state, transition); break;
            case 0x1: executeAction<0x1>(capture, captureIndex, _index, _state, transition); break;
            case 0x2: executeAction<0x2>(capture, captureIndex, _index, _state, transition); break;
            case 0x3: executeAction<0x3>(capture, captureIndex, _index, _state, transition); break;
            case 0x4: executeAction<0x4>(capture, captureIndex, _index, _state, transition); break;
            case 0x5: executeAction<0x5>(capture, captureIndex, _index, _state, transition); break;
            case 0x6: executeAction<0x6>(capture, captureIndex, _index, _state, transition); break;
            case 0x7: executeAction<0x7>(capture, captureIndex, _index, _state, transition); break;
            case 0x8: executeAction<0x8>(capture, captureIndex, _index, _state, transition); goto found;
            case 0x9: executeAction<0x9>(capture, captureIndex, _index, _state, transition); goto found;
            case 0xa: executeAction<0xa>(capture, captureIndex, _index, _state, transition); goto found;
            case 0xb: executeAction<0xb>(capture, captureIndex, _index, _state, transition); goto found;
            case 0xc: executeAction<0xc>(capture, captureIndex, _index, _state, transition); goto found;
            case 0xd: executeAction<0xd>(capture, captureIndex, _index, _state, transition); goto found;
            case 0xe: executeAction<0xe>(capture, captureIndex, _index, _state, transition); goto found;
            case 0xf: executeAction<0xf>(capture, captureIndex, _index, _state, transition); goto found;
            }
        }

        // Complete the token at the current state. Or an end-token at the initial state.
        if (_state == tokenizer_state_t::Initial) {
            // Mark the current offset as the position of the end-token.
            captureIndex = _index;
            capture.clear();
        }

        transition = transitionTable[get_offset(_state)];
        _state = transition.next();

    found:

        state = _state;
        index = _index;
        return {transition.name, std::string{capture.begin(), capture.end()}, captureIndex};
    }

    /*! Parse all tokens.
    */
    [[nodiscard]] std::vector<token_t> getTokens() noexcept {
        std::vector<token_t> r;

        tokenizer_name_t token_name;
        do {
            let token = getNextToken();
            token_name = token.name;
            r.push_back(std::move(token));
        } while (token_name != tokenizer_name_t::End);

        return r;
    }
};

[[nodiscard]] std::vector<token_t> parseTokens(std::string_view::const_iterator first, std::string_view::const_iterator last) noexcept
{
    return tokenizer(first, last).getTokens();
}

[[nodiscard]] std::vector<token_t> parseTokens(std::string_view text) noexcept
{
    return parseTokens(text.cbegin(), text.cend());
}

}