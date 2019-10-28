

#pragma once

#include "TTauri/Foundation/strings.hpp"
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

enum class tokenizer_state_t: uint8_t {
    Initial,
    Name,
    MinusOrPlus,            // Could be the start of a number, or an operator.
    Zero,                   // Could be part of a number with a base.
    Dot,                    // Could be the start of a floating point number, or an operator.
    Number,                 // Could be some kind of number without a base.
    Float,
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
constexpr size_t NR_TOKENIZER_STATE_VALUES = static_cast<size_t>(tokenizer_state_t::Sentinal);

enum class tokenizer_action_t: uint8_t {
    Idle = 0x00,
    Found = 0x01, // Token Found.
    Capture = 0x02, // Capture this character.
    Start = 0x04, // Start the capture queue.
    Read = 0x08, // Read next character, before processing next state.
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
    char c;
    tokenizer_state_t next;
    tokenizer_action_t action;
    tokenizer_name_t name;

    constexpr tokenizer_transition_t(
        char c = '\0',
        tokenizer_state_t next = tokenizer_state_t::Initial,
        tokenizer_action_t action = tokenizer_action_t::Idle,
        tokenizer_name_t name = tokenizer_name_t::ErrorNotAssigned
    ) : c(c), next(next), action(action), name(name) {}
};

namespace tokenizer_impl {


constexpr std::array<tokenizer_transition_t,256> calculateNameTransitionTable()
{
    std::array<tokenizer_transition_t,256> r{};

    for (uint16_t i = 0; i < 256; i++) {
        let c = static_cast<char>(i);
        tokenizer_transition_t transition = {c};

        if (isNameNext(c)) {
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

constexpr std::array<tokenizer_transition_t,256> calculateMinusOrPlusTransitionTable()
{
    std::array<tokenizer_transition_t,256> r{};

    for (uint16_t i = 0; i < 256; i++) {
        let c = static_cast<char>(i);
        tokenizer_transition_t transition = {c};

        if (c == '0') {
            transition.next = tokenizer_state_t::Zero;
            transition.action = tokenizer_action_t::Read | tokenizer_action_t::Capture;
        } else if (isDigit(c) || c == '.') {
            transition.next = tokenizer_state_t::Number;
        } else {
            transition.next = tokenizer_state_t::OperatorSecondChar;
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
            transition.next = tokenizer_state_t::Float;
        } else {
            transition.next = tokenizer_state_t::Initial;
            transition.action = tokenizer_action_t::Found;
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
            transition.next = tokenizer_state_t::Number;
            transition.action = tokenizer_action_t::Read | tokenizer_action_t::Capture;
        } else {
            transition.next = tokenizer_state_t::Number;
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
            transition.next = tokenizer_state_t::Number;
            transition.action = tokenizer_action_t::Read | tokenizer_action_t::Capture;
        } else if (c == '.') {
            transition.next = tokenizer_state_t::Float;
            transition.action = tokenizer_action_t::Read | tokenizer_action_t::Capture;
        } else if (c == '_' || c == '\'') {
            transition.next = tokenizer_state_t::Number;
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

constexpr std::array<tokenizer_transition_t,256> calculateFloatTransitionTable()
{
    std::array<tokenizer_transition_t,256> r{};

    for (uint16_t i = 0; i < 256; i++) {
        let c = static_cast<char>(i);
        tokenizer_transition_t transition = {c};

        if (isDigit(c) || c == 'e' || c == 'E' || c == '-') {
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

constexpr std::array<tokenizer_transition_t,256> calculateSlashTransitionTable()
{
    std::array<tokenizer_transition_t,256> r{};

    for (uint16_t i = 0; i < 256; i++) {
        let c = static_cast<char>(i);
        tokenizer_transition_t transition = {c};

        if (c == '/') {
            transition.next = tokenizer_state_t::LineComment;
            transition.action = tokenizer_action_t::Read;
        } else if (c == '*') {
            transition.next = tokenizer_state_t::BlockComment;
            transition.action = tokenizer_action_t::Read;
        } else {
            transition.next = tokenizer_state_t::OperatorFirstChar;
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
            transition.next = tokenizer_state_t::Initial;
        } else if (isLinefeed(c)) {
            transition.next = tokenizer_state_t::Initial;
            transition.action = tokenizer_action_t::Read;
        } else {
            transition.next = tokenizer_state_t::LineComment;
            transition.action = tokenizer_action_t::Read;
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
            transition.next = tokenizer_state_t::Initial;
            transition.action = tokenizer_action_t::Found;
            transition.name = tokenizer_name_t::ErrorEOTInBlockComment;
        } else if (c == '*') {
            transition.next = tokenizer_state_t::BlockCommentMaybeEnd;
            transition.action = tokenizer_action_t::Read;
        } else {
            transition.next = tokenizer_state_t::BlockComment;
            transition.action = tokenizer_action_t::Read;
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
            transition.action = tokenizer_action_t::Read;
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
            transition.next = tokenizer_state_t::Initial;
            transition.action = tokenizer_action_t::Found;
            transition.name = tokenizer_name_t::ErrorEOTInString;
        } else if (isLinefeed(c)) {
            transition.next = tokenizer_state_t::Initial;
            transition.action = tokenizer_action_t::Found | tokenizer_action_t::Read | tokenizer_action_t::Capture | tokenizer_action_t::Start;
            transition.name = tokenizer_name_t::ErrorLFInString;
        } else if (c == '\\') {
            transition.next = tokenizer_state_t::StringEscape;
            transition.action = tokenizer_action_t::Read;
        } else if (c == '"') {
            transition.next = tokenizer_state_t::Initial;
            transition.action = tokenizer_action_t::Found | tokenizer_action_t::Read;
            transition.name = tokenizer_name_t::StringLiteral;
        } else {
            transition.next = tokenizer_state_t::String;
            transition.action = tokenizer_action_t::Read | tokenizer_action_t::Capture;
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

        transition.next = tokenizer_state_t::String;
        transition.action = tokenizer_action_t::Read | tokenizer_action_t::Capture;
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
            transition.next = tokenizer_state_t::Initial;
            transition.action = tokenizer_action_t::Found | tokenizer_action_t::Read | tokenizer_action_t::Capture;
            transition.name = tokenizer_name_t::Literal;
        } else {
            transition.next = tokenizer_state_t::Initial;
            transition.action = tokenizer_action_t::Found;
        }

        r[i] = transition;
    }
    return r;
}

constexpr std::array<tokenizer_transition_t,256> calculateOperatorSecondCharTransitionTable()
{
#define LAST_CHAR\
    transition.next = tokenizer_state_t::Initial;\
    transition.action = tokenizer_action_t::Found | tokenizer_action_t::Read | tokenizer_action_t::Capture;\
    transition.name = tokenizer_name_t::Literal

#define MORE_CHARS\
    transition.next = tokenizer_state_t::OperatorThirdChar;\
    transition.action = tokenizer_action_t::Read | tokenizer_action_t::Capture;

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
            transition.next = tokenizer_state_t::Initial;
            transition.action = tokenizer_action_t::Found;
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
    transition.next = tokenizer_state_t::Initial;\
    transition.action = tokenizer_action_t::Found | tokenizer_action_t::Read | tokenizer_action_t::Capture;\
    transition.name = tokenizer_name_t::Literal

#define MORE_CHARS\
    transition.next = tokenizer_state_t::OperatorSecondChar;\
    transition.action = tokenizer_action_t::Read | tokenizer_action_t::Capture;

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

constexpr std::array<tokenizer_transition_t,256> calculateInitialTransitionTable()
{
    std::array<tokenizer_transition_t,256> r{};

    for (uint16_t i = 0; i < 256; i++) {
        let c = static_cast<char>(i);
        tokenizer_transition_t transition = {c};

        if (c == '\0') {
            transition.next = tokenizer_state_t::Initial;
            transition.action = tokenizer_action_t::Found;
            transition.name = tokenizer_name_t::End;
        } else if (isNameFirst(c)) {
            transition.next = tokenizer_state_t::Name;
            transition.action = tokenizer_action_t::Read | tokenizer_action_t::Capture | tokenizer_action_t::Start;
        } else if (c == '-' || c == '+') {
            transition.next = tokenizer_state_t::MinusOrPlus;
            transition.action = tokenizer_action_t::Read | tokenizer_action_t::Capture | tokenizer_action_t::Start;
        } else if (c == '0') {
            transition.next = tokenizer_state_t::Zero;
            transition.action = tokenizer_action_t::Read | tokenizer_action_t::Capture | tokenizer_action_t::Start;
        } else if (isDigit(c)) {
            transition.next = tokenizer_state_t::Number;
            transition.action = tokenizer_action_t::Read | tokenizer_action_t::Capture | tokenizer_action_t::Start;
        } else if (c == '.') {
            transition.next = tokenizer_state_t::Dot;
            transition.action = tokenizer_action_t::Read | tokenizer_action_t::Capture | tokenizer_action_t::Start;
        } else if (c == '"') {
            transition.next = tokenizer_state_t::String;
            transition.action = tokenizer_action_t::Read | tokenizer_action_t::Start;
        } else if (isWhitespace(c)) {
            transition.next = tokenizer_state_t::Initial;
            transition.action = tokenizer_action_t::Read;
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

constexpr size_t TRANSITION_TABLE_SIZE = NR_TOKENIZER_STATE_VALUES * 256;
constexpr std::array<tokenizer_transition_t,TRANSITION_TABLE_SIZE> calculateTransitionTable()
{
    std::array<tokenizer_transition_t,TRANSITION_TABLE_SIZE> r{};
    size_t i = 0;

    i = static_cast<size_t>(tokenizer_state_t::Initial) << 8;
    for (let t: calculateInitialTransitionTable()) { r[i++] = t; }

    i = static_cast<size_t>(tokenizer_state_t::Name) << 8;
    for (let t: calculateNameTransitionTable()) { r[i++] = t; }

    i = static_cast<size_t>(tokenizer_state_t::MinusOrPlus) << 8;
    for (let t: calculateMinusOrPlusTransitionTable()) { r[i++] = t; }

    i = static_cast<size_t>(tokenizer_state_t::Zero) << 8;
    for (let t: calculateZeroTransitionTable()) { r[i++] = t; }

    i = static_cast<size_t>(tokenizer_state_t::Dot) << 8;
    for (let t: calculateDotTransitionTable()) { r[i++] = t; }

    i = static_cast<size_t>(tokenizer_state_t::Number) << 8;
    for (let t: calculateNumberTransitionTable()) { r[i++] = t; }

    i = static_cast<size_t>(tokenizer_state_t::Float) << 8;
    for (let t: calculateFloatTransitionTable()) { r[i++] = t; }

    i = static_cast<size_t>(tokenizer_state_t::String) << 8;
    for (let t: calculateStringTransitionTable()) { r[i++] = t; }

    i = static_cast<size_t>(tokenizer_state_t::StringEscape) << 8;
    for (let t: calculateStringEscapeTransitionTable()) { r[i++] = t; }

    i = static_cast<size_t>(tokenizer_state_t::Slash) << 8;
    for (let t: calculateSlashTransitionTable()) { r[i++] = t; }

    i = static_cast<size_t>(tokenizer_state_t::LineComment) << 8;
    for (let t: calculateLineCommentTransitionTable()) { r[i++] = t; }

    i = static_cast<size_t>(tokenizer_state_t::BlockComment) << 8;
    for (let t: calculateBlockCommentTransitionTable()) { r[i++] = t; }

    i = static_cast<size_t>(tokenizer_state_t::BlockCommentMaybeEnd) << 8;
    for (let t: calculateBlockCommentMaybeEndTransitionTable()) { r[i++] = t; }

    i = static_cast<size_t>(tokenizer_state_t::OperatorFirstChar) << 8;
    for (let t: calculateOperatorFirstCharTransitionTable()) { r[i++] = t; }

    i = static_cast<size_t>(tokenizer_state_t::OperatorSecondChar) << 8;
    for (let t: calculateOperatorSecondCharTransitionTable()) { r[i++] = t; }

    i = static_cast<size_t>(tokenizer_state_t::OperatorThirdChar) << 8;
    for (let t: calculateOperatorThirdCharTransitionTable()) { r[i++] = t; }

    return r;
}

constexpr std::array<tokenizer_transition_t,TRANSITION_TABLE_SIZE> transitionTable = calculateTransitionTable();

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
    std::string capture;

    tokenizer(std::string_view text, size_t offset=0) :
        state(tokenizer_state_t::Initial), text(text), offset(offset), captureOffset(0), capture() {}

    /*! Parse a token.
     */
    no_inline tokenizer_token_t operator()() {
        tokenizer_token_t token;

        auto _state = state;
        auto _offset = offset;
        auto transition = tokenizer_transition_t{};
        while (_offset < text.size()) {
            let c = text[_offset];
            transition = tokenizer_impl::transitionTable[(static_cast<size_t>(_state) << 8) + c];

            if (transition.action >= tokenizer_action_t::Start) {
                capture.clear();
                captureOffset = _offset;
            }

            if (transition.action >= tokenizer_action_t::Capture) {
                capture += transition.c;
            }

            if (transition.action >= tokenizer_action_t::Read) {
                _offset++;
            }
 
            if (transition.action >= tokenizer_action_t::Found) {
                token = {transition.name, std::move(capture), captureOffset};
            }

            _state = transition.next;

            if (transition.action >= tokenizer_action_t::Found) {
                goto found;
            }
        }

        transition = tokenizer_impl::transitionTable[static_cast<uint16_t>(_state) << 8 | 0];
        token = {transition.name, std::move(capture), offset};
        _state = tokenizer_state_t::Initial;
found:
        state = _state;
        offset = _offset;
        return token;
    }
};


}

