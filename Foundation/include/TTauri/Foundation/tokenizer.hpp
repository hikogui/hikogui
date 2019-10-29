

#pragma once

#include "TTauri/Foundation/strings.hpp"
#include "TTauri/Foundation/small_vector.hpp"
#include "TTauri/Foundation/required.hpp"
#include <memory>
#include <string>
#include <string_view>
#include <charconv>
#include <array>

namespace TTauri {

enum class tokenizer_name_t : uint8_t {
    ErrorNotAssigned,
    ErrorInvalidCharacter,
    ErrorEOTInBlockComment,
    ErrorEOTInString,
    ErrorLFInString,

    Operator,
    Name,
    StringLiteral,
    IntegerLiteral,
    FloatLiteral,
    Literal,                // Operator, or bracket, or other literal text.
    End
};

enum class tokenizer_state_t: uint16_t {
    Initial = 0x0000,
    Name = 0x0100,
    MinusOrPlus = 0x0200,            // Could be the start of a number, or an operator.
    Zero = 0x0300,                   // Could be part of a number with a base.
    Dot = 0x0400,                    // Could be the start of a floating point number, or an operator.
    Number = 0x0500,                 // Could be some kind of number without a base.
    Float = 0x0600,
    String = 0x0700,
    StringEscape = 0x0800,
    Slash = 0x0900,                  // Could be the start of a LineComment, BlockComment, or an operator.
    LineComment = 0x0a00,
    BlockComment = 0x0b00,
    BlockCommentMaybeEnd = 0x0c00,   // Found a '*' possibly end of comment.
    OperatorFirstChar = 0x0d00,
    OperatorSecondChar = 0x0e00,
    OperatorThirdChar = 0x0f00,

    Sentinal = 0x1000
};
constexpr size_t NR_TOKENIZER_STATE_VALUES = static_cast<size_t>(tokenizer_state_t::Sentinal) >> 8;

enum class tokenizer_action_t: uint8_t {
    Idle = 0x00,
    Capture = 0x01, // Capture this character.
    Start = 0x02, // Start the capture queue.
    Read = 0x04, // Read next character, before processing next state.
    Found = 0x08, // Token Found.
};

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
        char c = '\0',
        tokenizer_state_t next = tokenizer_state_t::Initial,
        tokenizer_action_t action = tokenizer_action_t::Idle,
        tokenizer_name_t name = tokenizer_name_t::ErrorNotAssigned
    ) :
        actionAndNextState(static_cast<uint16_t>(next) | static_cast<uint16_t>(action)),
        c(c),
        name(name) {}

    constexpr tokenizer_state_t next() const noexcept {
        uint16_t stateInt = actionAndNextState & 0xff00;
        axiom_assert(stateInt < static_cast<uint16_t>(tokenizer_state_t::Sentinal));
        return static_cast<tokenizer_state_t>(stateInt);
    }
    constexpr tokenizer_action_t action() const noexcept {
        uint8_t actionInt = static_cast<uint8_t>(actionAndNextState);
        axiom_assert(actionInt <= 0x0f);
        return static_cast<tokenizer_action_t>(actionInt);
    }

    constexpr void setNext(tokenizer_state_t state) noexcept {
        uint16_t stateInt = static_cast<uint16_t>(state);
        axiom_assert((stateInt & 0xff) == 0);
        actionAndNextState &= 0x00ff;
        actionAndNextState |= static_cast<uint16_t>(state);
    }

    constexpr void setAction(tokenizer_action_t action) noexcept {
        actionAndNextState &= 0xff00;
        actionAndNextState |= static_cast<uint16_t>(action);
    }
};

namespace tokenizer_impl {


constexpr std::array<tokenizer_transition_t,256> calculateNameTransitionTable()
{
    std::array<tokenizer_transition_t,256> r{};

    for (uint16_t i = 0; i < 256; i++) {
        let c = static_cast<char>(i);
        tokenizer_transition_t transition = {c};

        if (isNameNext(c)) {
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

constexpr std::array<tokenizer_transition_t,256> calculateMinusOrPlusTransitionTable()
{
    std::array<tokenizer_transition_t,256> r{};

    for (uint16_t i = 0; i < 256; i++) {
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

constexpr std::array<tokenizer_transition_t,256> calculateDotTransitionTable()
{
    std::array<tokenizer_transition_t,256> r{};

    for (uint16_t i = 0; i < 256; i++) {
        let c = static_cast<char>(i);
        tokenizer_transition_t transition = {c};

        if (isDigit(c)) {
            transition.setNext(tokenizer_state_t::Float);
        } else {
            transition.setNext(tokenizer_state_t::Initial);
            transition.setAction(tokenizer_action_t::Found);
            transition.name = tokenizer_name_t::Literal;
        }

        r[i] = transition;
    }
    return r;
}

constexpr std::array<tokenizer_transition_t,256> calculateZeroTransitionTable()
{
    std::array<tokenizer_transition_t,256> r{};

    for (uint16_t i = 0; i < 256; i++) {
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

constexpr std::array<tokenizer_transition_t,256> calculateNumberTransitionTable()
{
    std::array<tokenizer_transition_t,256> r{};

    for (uint16_t i = 0; i < 256; i++) {
        let c = static_cast<char>(i);
        tokenizer_transition_t transition = {c};

        if (isDigit(c)) {
            transition.setNext(tokenizer_state_t::Number);
            transition.setAction(tokenizer_action_t::Read | tokenizer_action_t::Capture);
        } else if (c == '.') {
            transition.setNext(tokenizer_state_t::Float);
            transition.setAction(tokenizer_action_t::Read | tokenizer_action_t::Capture);
        } else if (c == '_' || c == '\'') {
            transition.setNext(tokenizer_state_t::Number);
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

constexpr std::array<tokenizer_transition_t,256> calculateFloatTransitionTable()
{
    std::array<tokenizer_transition_t,256> r{};

    for (uint16_t i = 0; i < 256; i++) {
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

constexpr std::array<tokenizer_transition_t,256> calculateSlashTransitionTable()
{
    std::array<tokenizer_transition_t,256> r{};

    for (uint16_t i = 0; i < 256; i++) {
        let c = static_cast<char>(i);
        tokenizer_transition_t transition = {c};

        if (c == '/') {
            transition.setNext(tokenizer_state_t::LineComment);
            transition.setAction(tokenizer_action_t::Read);
        } else if (c == '*') {
            transition.setNext(tokenizer_state_t::BlockComment);
            transition.setAction(tokenizer_action_t::Read);
        } else {
            transition.setNext(tokenizer_state_t::OperatorFirstChar);
        }

        r[i] = transition;
    }
    return r;
}

constexpr std::array<tokenizer_transition_t,256> calculateLineCommentTransitionTable()
{
    std::array<tokenizer_transition_t,256> r{};

    for (uint16_t i = 0; i < 256; i++) {
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

constexpr std::array<tokenizer_transition_t,256> calculateBlockCommentTransitionTable()
{
    std::array<tokenizer_transition_t,256> r{};

    for (uint16_t i = 0; i < 256; i++) {
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

constexpr std::array<tokenizer_transition_t,256> calculateBlockCommentMaybeEndTransitionTable()
{
    std::array<tokenizer_transition_t,256> r{};

    for (uint16_t i = 0; i < 256; i++) {
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

constexpr std::array<tokenizer_transition_t,256> calculateStringTransitionTable()
{
    std::array<tokenizer_transition_t,256> r{};

    for (uint16_t i = 0; i < 256; i++) {
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

constexpr std::array<tokenizer_transition_t,256> calculateStringEscapeTransitionTable()
{
    std::array<tokenizer_transition_t,256> r{};

    for (uint16_t i = 0; i < 256; i++) {
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

constexpr std::array<tokenizer_transition_t,256> calculateOperatorThirdCharTransitionTable()
{
    std::array<tokenizer_transition_t,256> r{};

    for (uint16_t i = 0; i < 256; i++) {
        let c = static_cast<char>(i);
        tokenizer_transition_t transition = {c};

        if (c == '>') {
            transition.setNext(tokenizer_state_t::Initial);
            transition.setAction(tokenizer_action_t::Found | tokenizer_action_t::Read | tokenizer_action_t::Capture);
            transition.name = tokenizer_name_t::Literal;
        } else {
            transition.setNext(tokenizer_state_t::Initial);
            transition.setAction(tokenizer_action_t::Found);
        }

        r[i] = transition;
    }
    return r;
}

constexpr std::array<tokenizer_transition_t,256> calculateOperatorSecondCharTransitionTable()
{
#define LAST_CHAR\
    transition.setNext(tokenizer_state_t::Initial);\
    transition.setAction(tokenizer_action_t::Found | tokenizer_action_t::Read | tokenizer_action_t::Capture);\
    transition.name = tokenizer_name_t::Literal

#define MORE_CHARS\
    transition.setNext(tokenizer_state_t::OperatorThirdChar);\
    transition.setAction(tokenizer_action_t::Read | tokenizer_action_t::Capture);

    std::array<tokenizer_transition_t,256> r{};

    for (uint16_t i = 0; i < 256; i++) {
        let c = static_cast<char>(i);
        tokenizer_transition_t transition = {c};

        switch (c) {
        case '=': MORE_CHARS; break; // Possible: <=>

        case '-': LAST_CHAR; break;
        case '+': LAST_CHAR; break;
        case '*': LAST_CHAR; break;
        case '&': LAST_CHAR; break;
        case '|': LAST_CHAR; break;
        case '^': LAST_CHAR; break;
        case '<': LAST_CHAR; break;
        case '>': LAST_CHAR; break;
        default:
            transition.setNext(tokenizer_state_t::Initial);
            transition.setAction(tokenizer_action_t::Found);
        }

        r[i] = transition;
    }
    return r;
#undef LAST_CHAR
#undef MORE_CHARS
}

constexpr std::array<tokenizer_transition_t,256> calculateOperatorFirstCharTransitionTable()
{
#define LAST_CHAR\
    transition.setNext(tokenizer_state_t::Initial);\
    transition.setAction(tokenizer_action_t::Found | tokenizer_action_t::Read | tokenizer_action_t::Capture);\
    transition.name = tokenizer_name_t::Literal

#define MORE_CHARS\
    transition.setNext(tokenizer_state_t::OperatorSecondChar);\
    transition.setAction(tokenizer_action_t::Read | tokenizer_action_t::Capture);

    std::array<tokenizer_transition_t,256> r{};

    for (uint16_t i = 0; i < 256; i++) {
        let c = static_cast<char>(i);
        tokenizer_transition_t transition = {c};

        switch (c) {
        case '.': LAST_CHAR; break;
        case ';': LAST_CHAR; break;
        case ',': LAST_CHAR; break;
        case '/': LAST_CHAR; break;
        case '(': LAST_CHAR; break;
        case ')': LAST_CHAR; break;
        case '[': LAST_CHAR; break;
        case ']': LAST_CHAR; break;
        case '{': LAST_CHAR; break;
        case '}': LAST_CHAR; break;
        case '?': LAST_CHAR; break;
        case '%': LAST_CHAR; break;
        case '@': LAST_CHAR; break;
        case '$': LAST_CHAR; break;
        case '~': LAST_CHAR; break;

        case '!': MORE_CHARS; break; // Possible: !=
        case '<': MORE_CHARS; break; // Possible: <=>, <=, <-, <<, <>
        case '>': MORE_CHARS; break; // Possible: >=, >>
        case '=': MORE_CHARS; break; // Possible: ==, =>
        case '+': MORE_CHARS; break; // Possible: ++
        case '-': MORE_CHARS; break; // Possible: --, ->,
        case '*': MORE_CHARS; break; // Possible: **
        case '|': MORE_CHARS; break; // Possible: ||
        case '&': MORE_CHARS; break; // Possible: &&
        case '^': MORE_CHARS; break; // Possible: ^^
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

constexpr std::array<tokenizer_transition_t,256> calculateInitialTransitionTable()
{
    std::array<tokenizer_transition_t,256> r{};

    for (uint16_t i = 0; i < 256; i++) {
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

constexpr size_t TRANSITION_TABLE_SIZE = NR_TOKENIZER_STATE_VALUES * 256;
using transitionTable_t = std::array<tokenizer_transition_t,TRANSITION_TABLE_SIZE>;

constexpr bool optimizeTransitionTable(transitionTable_t &r)
{
    bool foundOptimization = false;
    for (size_t state = 0; state < TRANSITION_TABLE_SIZE; state += 256) {
        for (size_t c = 0; c < 256; c++) {
            auto &transition = r[state | c];
            if (transition.action() == tokenizer_action_t::Idle) {
                foundOptimization = true;

                let next_state = static_cast<uint16_t>(transition.next());
                transition = r[next_state | c];
            }
        }
    }
    return foundOptimization;
}

constexpr transitionTable_t calculateTransitionTable()
{
    transitionTable_t r{};
    size_t i = 0;

    i = static_cast<size_t>(tokenizer_state_t::Initial);
    for (let t: calculateInitialTransitionTable()) { r[i++] = t; }

    i = static_cast<size_t>(tokenizer_state_t::Name);
    for (let t: calculateNameTransitionTable()) { r[i++] = t; }

    i = static_cast<size_t>(tokenizer_state_t::MinusOrPlus);
    for (let t: calculateMinusOrPlusTransitionTable()) { r[i++] = t; }

    i = static_cast<size_t>(tokenizer_state_t::Zero);
    for (let t: calculateZeroTransitionTable()) { r[i++] = t; }

    i = static_cast<size_t>(tokenizer_state_t::Dot);
    for (let t: calculateDotTransitionTable()) { r[i++] = t; }

    i = static_cast<size_t>(tokenizer_state_t::Number);
    for (let t: calculateNumberTransitionTable()) { r[i++] = t; }

    i = static_cast<size_t>(tokenizer_state_t::Float);
    for (let t: calculateFloatTransitionTable()) { r[i++] = t; }

    i = static_cast<size_t>(tokenizer_state_t::String);
    for (let t: calculateStringTransitionTable()) { r[i++] = t; }

    i = static_cast<size_t>(tokenizer_state_t::StringEscape);
    for (let t: calculateStringEscapeTransitionTable()) { r[i++] = t; }

    i = static_cast<size_t>(tokenizer_state_t::Slash);
    for (let t: calculateSlashTransitionTable()) { r[i++] = t; }

    i = static_cast<size_t>(tokenizer_state_t::LineComment);
    for (let t: calculateLineCommentTransitionTable()) { r[i++] = t; }

    i = static_cast<size_t>(tokenizer_state_t::BlockComment);
    for (let t: calculateBlockCommentTransitionTable()) { r[i++] = t; }

    i = static_cast<size_t>(tokenizer_state_t::BlockCommentMaybeEnd);
    for (let t: calculateBlockCommentMaybeEndTransitionTable()) { r[i++] = t; }

    i = static_cast<size_t>(tokenizer_state_t::OperatorFirstChar);
    for (let t: calculateOperatorFirstCharTransitionTable()) { r[i++] = t; }

    i = static_cast<size_t>(tokenizer_state_t::OperatorSecondChar);
    for (let t: calculateOperatorSecondCharTransitionTable()) { r[i++] = t; }

    i = static_cast<size_t>(tokenizer_state_t::OperatorThirdChar);
    for (let t: calculateOperatorThirdCharTransitionTable()) { r[i++] = t; }

    while (optimizeTransitionTable(r)) {}
    return r;
}

constexpr transitionTable_t transitionTable = calculateTransitionTable();

}



struct tokenizer_token_t {
    tokenizer_name_t name;
    std::string value;
    size_t offset;
};

inline bool operator==(tokenizer_token_t const &lhs, tokenizer_token_t const &rhs) noexcept
{
    return (lhs.name == rhs.name) && (lhs.value == rhs.value) && (lhs.offset == rhs.offset);
}



/*! Generic tokenizer.
* This tokenizer is for parsing tokens in most languages.
* It will recognize:
*    - integers literal
*    - floating point literal
*    - string literal
*    - boolean literal
*    - null
*    - names
*    - operators
*    - comments
*    - white space
*/
struct tokenizer {
    tokenizer_state_t state;
    std::string_view text;
    size_t offset;
    size_t captureOffset;

    tokenizer(std::string_view text, size_t offset=0) :
        state(tokenizer_state_t::Initial), text(text), offset(offset), captureOffset(0) {}


    template<uint8_t Action>
    static force_inline void executeAction(
        small_vector<char,256> &capture,
        size_t &captureOffset,
        size_t &offset,
        tokenizer_state_t &state,
        tokenizer_transition_t &transition
    ) {
        constexpr auto action = static_cast<tokenizer_action_t>(Action);
        if constexpr (action >= tokenizer_action_t::Start) {
            captureOffset = offset;
            capture.clear();
        }

        if constexpr (action >= tokenizer_action_t::Capture) {
            capture.push_back(transition.c);
        }

        if constexpr (action >= tokenizer_action_t::Read) {
            offset++;
        }

        state = transition.next();
    }

    /*! Parse a token.
     */
    no_inline tokenizer_token_t operator()() {
        small_vector<char,256> capture;

        auto _state = state;
        auto _offset = offset;
        auto transition = tokenizer_transition_t{};
        while (_offset < text.size()) {
            let c = text[_offset];
            transition = tokenizer_impl::transitionTable[static_cast<size_t>(_state) | c];

            switch (static_cast<uint8_t>(transition.action())) {
            case 0x0: executeAction<0x0>(capture, captureOffset, _offset, _state, transition); break;
            case 0x1: executeAction<0x1>(capture, captureOffset, _offset, _state, transition); break;
            case 0x2: executeAction<0x2>(capture, captureOffset, _offset, _state, transition); break;
            case 0x3: executeAction<0x3>(capture, captureOffset, _offset, _state, transition); break;
            case 0x4: executeAction<0x4>(capture, captureOffset, _offset, _state, transition); break;
            case 0x5: executeAction<0x5>(capture, captureOffset, _offset, _state, transition); break;
            case 0x6: executeAction<0x6>(capture, captureOffset, _offset, _state, transition); break;
            case 0x7: executeAction<0x7>(capture, captureOffset, _offset, _state, transition); break;
            case 0x8: executeAction<0x8>(capture, captureOffset, _offset, _state, transition); goto found;
            case 0x9: executeAction<0x9>(capture, captureOffset, _offset, _state, transition); goto found;
            case 0xa: executeAction<0xa>(capture, captureOffset, _offset, _state, transition); goto found;
            case 0xb: executeAction<0xb>(capture, captureOffset, _offset, _state, transition); goto found;
            case 0xc: executeAction<0xc>(capture, captureOffset, _offset, _state, transition); goto found;
            case 0xd: executeAction<0xd>(capture, captureOffset, _offset, _state, transition); goto found;
            case 0xe: executeAction<0xe>(capture, captureOffset, _offset, _state, transition); goto found;
            case 0xf: executeAction<0xf>(capture, captureOffset, _offset, _state, transition); goto found;
            }
        }

        // Complete the token at the current state. Or an end-token at the initial state.
        if (_state == tokenizer_state_t::Initial) {
            // Mark the current offset as the position of the end-token.
            captureOffset = _offset;
        }

        transition = tokenizer_impl::transitionTable[static_cast<uint16_t>(_state)];
found:

        state = _state;
        offset = _offset;
        return {transition.name, std::string{capture.begin(), capture.end()}, captureOffset};
    }
};


}

