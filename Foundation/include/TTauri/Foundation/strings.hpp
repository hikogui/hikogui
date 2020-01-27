// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/algorithm.hpp"
#include "TTauri/Foundation/numeric_cast.hpp"
#include "TTauri/Foundation/required.hpp"
#include "TTauri/Foundation/assert.hpp"
#include "TTauri/Foundation/os_detect.hpp"
#include <string>
#include <string_view>
#include <iterator>
#include <vector>
#include <tuple>

namespace TTauri {

[[nodiscard]] constexpr bool isUpper(char c) noexcept {
    return c >= 'A' && c <= 'Z';
}

[[nodiscard]] constexpr bool isLower(char c) noexcept {
    return c >= 'a' && c <= 'z';
}

[[nodiscard]] constexpr bool isAlpha(char c) noexcept {
    return isUpper(c) || isLower(c);
}

[[nodiscard]] constexpr bool isDigit(char c) noexcept {
    return c >= '0' && c <= '9';
}

[[nodiscard]] constexpr bool isAlphaNum(char c) noexcept {
    return isAlpha(c) || isDigit(c);
}

[[nodiscard]] constexpr bool isLinefeed(char c) noexcept {
    return c == '\r' || c == '\n' || c == '\f' || c == '\v';
}

[[nodiscard]] constexpr bool isWhitespace(char c) noexcept {
    return c == ' ' || c == '\t' || isLinefeed(c);
}

[[nodiscard]] constexpr bool isNumberFirst(char c) noexcept {
    return isDigit(c) || c == '+' || c == '-';
}

[[nodiscard]] constexpr bool isNameFirst(char c) noexcept {
    return isAlpha(c) || c == '_' || c == '$';
}

[[nodiscard]] constexpr bool isNameNext(char c) noexcept {
    return isAlphaNum(c) || c == '_' || c == '$';
}

[[nodiscard]] constexpr bool isQuote(char c) noexcept {
    return c == '"' || c == '\'' || c == '`';
}

[[nodiscard]] constexpr bool isOpenBracket(char c) noexcept {
    return c == '(' || c == '{' || c == '[';
}

[[nodiscard]] constexpr bool isCloseBracket(char c) noexcept {
    return c == ')' || c == '}' || c == ']';
}

[[nodiscard]] constexpr bool isOperator(char c) noexcept {
    return
        !isAlphaNum(c) && c != '_' &&
        !isWhitespace(c) &&
        !isQuote(c) &&
        !isOpenBracket(c) &&
        !isCloseBracket(c);
}

[[nodiscard]] inline std::string to_lower(std::string_view str) noexcept
{
    std::string r;
    r.reserve(size(str));

    for (let c: str) {
        r += (c >= 'A' && c <= 'Z') ? (c - 'A') + 'a': c;
    }

    return r;
}

[[nodiscard]] inline std::string to_upper(std::string_view str) noexcept
{
    std::string r;
    r.reserve(size(str));

    for (let c: str) {
        r += (c >= 'a' && c <= 'z') ? (c - 'a') + 'A': c;
    }

    return r;
}

/** Normalize string to use only line-feeds.
 */
[[nodiscard]] inline std::string normalize_lf(std::string_view str) noexcept
{
    std::string r;
    r.reserve(size(str));

    auto found_cr = false;
    for (let c: str) {
        if (ttauri_unlikely(found_cr)) {
            // This is Microsoft or old-Apple, we replace the previous carriage-return
            // with a line-feed and emit the current character.
            r += '\n';
            if (c != '\r' && c != '\n') {
                r += c;
            }
        
        } else if (ttauri_likely(c != '\r')) {
            // Emit any non-carriage return character.
            r += c;
        }

        found_cr = c == '\r';
    }
    if (found_cr) {
        r += '\n';
    }

    return r;
}

/** Encode a string to be usable as an id.
 * An id has the following format: [_a-zA-Z][_a-zA-Z0-9]*
 */
[[nodiscard]] inline std::string id_encode(std::string_view str) noexcept
{
    std::string r;
    r.reserve(size(str));

    r += isNameFirst(str.front()) ? str.front() : '_';
    for (let c: str.substr(1)) {
        r += isNameNext(c) ? c : '_';
    }

    return r;
}

gsl_suppress3(f.23,bounds.1,bounds.3)
[[nodiscard]] constexpr uint32_t fourcc(char const txt[5]) noexcept
{
    return (
        (static_cast<uint32_t>(txt[0]) << 24) |
        (static_cast<uint32_t>(txt[1]) << 16) |
        (static_cast<uint32_t>(txt[2]) <<  8) |
        static_cast<uint32_t>(txt[3])
        );
}

gsl_suppress(bounds.3)
[[nodiscard]] inline std::string fourcc_to_string(uint32_t x) noexcept
{
    char c_str[5];
    c_str[0] = numeric_cast<char>((x >> 24) & 0xff);
    c_str[1] = numeric_cast<char>((x >> 16) & 0xff);
    c_str[2] = numeric_cast<char>((x >> 8) & 0xff);
    c_str[3] = numeric_cast<char>(x & 0xff);
    c_str[4] = 0;

    return {c_str};
}

[[nodiscard]] constexpr char nibble_to_char(uint8_t nibble) noexcept
{
    if (nibble <= 9) {
        return '0' + nibble;
    } else if (nibble <= 15) {
        return 'a' + nibble - 10;
    } else {
        no_default;
    }
}

/*!
 * \return value between 0-15, or -1 on error.
 */
[[nodiscard]] inline int8_t char_to_nibble(char c) noexcept
{
    if (c >= '0' && c <= '9') {
        return c - '0';
    } else if (c >= 'a' && c <= 'f') {
        return (c - 'a') + 10;
    } else if (c >= 'A' && c <= 'F') {
        return (c - 'A') + 10;
    } else {
        return -1;
    }
}

[[nodiscard]] inline std::string_view make_string_view(
    typename std::string::const_iterator b,
    typename std::string::const_iterator e
) noexcept
{
    return (b != e) ?
        std::string_view{&(*b), numeric_cast<size_t>(std::distance(b, e))} :
        std::string_view{};
}

template<typename T, typename... Args>
[[nodiscard]] inline std::vector<T> split(T haystack, Args... needle) noexcept
{
    std::vector<T> r;

    size_t offset = 0;
    size_t pos = std::min({haystack.find(needle, offset)...});
    while (pos != haystack.npos) {
        r.push_back(haystack.substr(offset, pos - offset));

        offset = pos + 1;
        pos = std::min({haystack.find(needle, offset)...});
    }

    r.push_back(haystack.substr(offset, haystack.size() - offset));
    return r;
}

[[nodiscard]] inline std::string join(std::vector<std::string> const &list, std::string_view const joiner = {}) noexcept
{
    std::string r;

    if (list.size() > 1) {
        size_t final_size = (list.size() - 1) * joiner.size();
        for (let &item: list) {
            final_size += item.size();
        }
        r.reserve(final_size);
    }

    int64_t i = 0;
    for (let &item: list) {
        if (i++ > 0) {
            r += joiner;
        }
        r += item;
    }
    return r;
}

[[nodiscard]] inline std::string join(std::vector<std::string_view> const &list, std::string_view const joiner = {}) noexcept
{
    std::string r;

    if (list.size() > 1) {
        size_t final_size = (list.size() - 1) * joiner.size();
        for (let &item: list) {
            final_size += item.size();
        }
        r.reserve(final_size);
    }

    int64_t i = 0;
    for (let &item: list) {
        if (i++ > 0) {
            r += joiner;
        }
        r += item;
    }
    return r;
}

constexpr char32_t UNICODE_Replacement_Character = 0xfffd;
constexpr char32_t UNICODE_Surrogates_BEGIN = 0xd800;
constexpr char32_t UNICODE_Surrogates_END = 0xdfff;
constexpr char32_t UNICODE_High_Surrogates_BEGIN = 0xd800;
constexpr char32_t UNICODE_High_Surrogates_END = 0xdbff;
constexpr char32_t UNICODE_Low_Surrogates_BEGIN = 0xdc00;
constexpr char32_t UNICODE_Low_Surrogates_END = 0xdfff;
constexpr char32_t UNICODE_ASCII_END = 0x7f;
constexpr char32_t UNICODE_Plane_0_END = 0xffff;
constexpr char32_t UNICODE_Basic_Multilinqual_Plane_END = UNICODE_Plane_0_END;
constexpr char32_t UNICODE_Plane_1_BEGIN = 0x010000;
constexpr char32_t UNICODE_Plane_16_END = 0x10ffff;
constexpr char32_t UNICODE_Plane_17_BEGIN = 0x110000;
constexpr char32_t UNICODE_Zero_Width_No_Break_Space = 0xfeff;
constexpr char32_t UNICODE_BOM = UNICODE_Zero_Width_No_Break_Space;
constexpr char32_t UNICODE_Reverse_BOM = 0xfffe;

[[nodiscard]] inline char32_t CP1252ToCodePoint(uint8_t inputCharacter) noexcept
{
    if (inputCharacter >= 0 && inputCharacter <= 0x7f) {
        return inputCharacter;
    } else if (inputCharacter >= 0xa0 && inputCharacter <= 0xff) {
        return inputCharacter;
    } else {
        switch (inputCharacter) {
        case 0x80: return 0x20ac;
        case 0x81: return UNICODE_Replacement_Character;
        case 0x82: return 0x201a;
        case 0x83: return 0x0192;
        case 0x84: return 0x201e;
        case 0x85: return 0x2026;
        case 0x86: return 0x2020;
        case 0x87: return 0x2021;
        case 0x88: return 0x02c6;
        case 0x89: return 0x2030;
        case 0x8a: return 0x0160;
        case 0x8b: return 0x2039;
        case 0x8c: return 0x0152;
        case 0x8d: return UNICODE_Replacement_Character;
        case 0x8e: return 0x017d;
        case 0x8f: return UNICODE_Replacement_Character;
        case 0x90: return UNICODE_Replacement_Character;
        case 0x91: return 0x2018;
        case 0x92: return 0x2019;
        case 0x93: return 0x201c;
        case 0x94: return 0x201d;
        case 0x95: return 0x2022;
        case 0x96: return 0x2013;
        case 0x97: return 0x2014;
        case 0x98: return 0x02dc;
        case 0x99: return 0x2122;
        case 0x9a: return 0x0161;
        case 0x9b: return 0x203a;
        case 0x9c: return 0x0153;
        case 0x9d: return UNICODE_Replacement_Character;
        case 0x9e: return 0x017e;
        case 0x9f: return 0x0178;
        default: return UNICODE_Replacement_Character;
        }
    }
}

struct TranslateStringOptions {
    bool _allowCP1252 = false;
    bool _allowSurrogate = false;
    bool _byteSwap = false;
    bool _addBOM = false;

    TranslateStringOptions &allowCP1252(bool allowCP1252_value = true) noexcept {
        _allowCP1252 = allowCP1252_value;
        return *this;
    }
    TranslateStringOptions &allowSurrogate(bool allowSurrogate_value = true) noexcept {
        _allowSurrogate = allowSurrogate_value;
        return *this;
    }
    TranslateStringOptions &byteSwap(bool byteSwap_value = true) noexcept {
        _byteSwap = byteSwap_value;
        return *this;
    }
    TranslateStringOptions &addBOM(bool addBOM_value = true) noexcept {
        _addBOM = addBOM_value;
        return *this;
    }
};

template<typename T, typename U>
[[nodiscard]] inline T translateString(U const inputString, TranslateStringOptions options = {}) noexcept
{
    if constexpr (sizeof (typename T::value_type) == sizeof (typename U::value_type)) {
        return transform<T>(inputString, [](let &inputCharacter) { return inputCharacter; });
    } else {
        let intermediateString = translateString<std::u32string>(inputString, options);
        return translateString<T>(intermediateString, options);
    }
}

template<typename T, typename U>
[[nodiscard]] inline T translateString(std::basic_string<U> const inputString, TranslateStringOptions options = {}) noexcept {
    return translateString<T>(std::basic_string_view<U>(inputString), options);
}

template<>
[[nodiscard]] inline std::u32string translateString(std::string_view const inputString, TranslateStringOptions options) noexcept
{
    std::u32string outputString;
    char32_t codePoint = 0;
    size_t codePointToDo = 1;
    size_t backtrackPosition = 0;

    for (size_t i = 0; i < inputString.size(); i++) {
        let inputCharacter = static_cast<uint8_t>(inputString.at(i));

        if (codePointToDo == 1) {
            backtrackPosition = i;

            if ((inputCharacter & 0x80) == 0x00) {
                codePoint = inputCharacter;
                codePointToDo = 1;

            } else if ((inputCharacter & 0xe0) == 0xc0) {
                codePoint = (inputCharacter & 0x1f);
                codePointToDo = 2;

            } else if ((inputCharacter & 0xf0) == 0xe0) {
                codePoint = (inputCharacter & 0x0f);
                codePointToDo = 3;

            } else if ((inputCharacter & 0xf8) == 0xf0) {
                codePoint = (inputCharacter & 0x07);
                codePointToDo = 4;

            } else if (inputCharacter > 0xfe) {
                // UTF-16 bytecode mark should not appear in UTF-8.
                codePoint = UNICODE_Replacement_Character;
                codePointToDo = 1;

            } else {
                // Invalid UTF-8 byte value.
                codePoint = 0x40000000 | inputCharacter;
                codePointToDo = 1;
            }

        } else if ((inputCharacter & 0xc0) == 0x80) {
            codePoint = (codePoint << 6) | (inputCharacter & 0x3f);
            codePointToDo--;

        } else {
            // Error decoding a multibyte code point, backtrack and report the single byte.
            codePoint = 0x40000000 | inputString.at(backtrackPosition);
            codePointToDo = 1;
            i = backtrackPosition;
        }

        if (codePointToDo == 1) {
            if (codePoint >= 0x40000000 && options._allowCP1252) {
                outputString.push_back(CP1252ToCodePoint(codePoint & 0xff));

            } else if (codePoint >= UNICODE_Surrogates_BEGIN && codePoint <= UNICODE_Surrogates_END && !options._allowSurrogate) {
                outputString.push_back(UNICODE_Replacement_Character);

            } else if (codePoint >= UNICODE_Plane_17_BEGIN) {
                outputString.push_back(UNICODE_Replacement_Character);

            } else {
                outputString.push_back(codePoint);
            }
        }
    }

    return outputString;
}

template<>
[[nodiscard]] inline std::u32string translateString(std::u16string_view const inputString, TranslateStringOptions options) noexcept
{
    bool byteSwap = options._byteSwap;
    std::u32string outputString;
    char16_t firstSurrogate = 0;

    for (size_t i = 0; i < inputString.size(); i++) {
        auto inputCharacter = inputString.at(i);
        if (byteSwap) {
            inputCharacter = (inputCharacter >> 8) | (inputCharacter & 0xff);
        }

        if (i == 0 && inputCharacter == UNICODE_BOM) {
            // Ignore correct BOM

        } else if (i == 0 && inputCharacter == UNICODE_Reverse_BOM) {
            // Incorrect BOM.
            byteSwap = !byteSwap;

        } else if (firstSurrogate && inputCharacter >= UNICODE_Low_Surrogates_BEGIN && inputCharacter <= UNICODE_Low_Surrogates_END) {
            // Second surrogate
            char32_t codePoint = firstSurrogate - UNICODE_High_Surrogates_BEGIN;
            codePoint <<= 10;
            codePoint |= inputCharacter - UNICODE_Low_Surrogates_BEGIN;
            codePoint += UNICODE_Plane_1_BEGIN;

            outputString.push_back(codePoint);
            firstSurrogate = 0;

        } else {
            if (firstSurrogate) {
                // Incomplete surrogate pair.
                outputString.push_back(options._allowSurrogate ? firstSurrogate : UNICODE_Replacement_Character);
                firstSurrogate = 0;
            }

            if (inputCharacter >= UNICODE_High_Surrogates_BEGIN && inputCharacter <= UNICODE_High_Surrogates_END) {
                // First surrogate.
                firstSurrogate = inputCharacter;

            } else if (inputCharacter >= UNICODE_Low_Surrogates_BEGIN && inputCharacter <= UNICODE_Low_Surrogates_END) {
                // Out-of-order surrogate.
                outputString.push_back(options._allowSurrogate ? inputCharacter : UNICODE_Replacement_Character);

            } else  {
                // Normal characters.
                outputString.push_back(inputCharacter);
            }
        }
    }

    return outputString;
}

template<>
[[nodiscard]] inline std::u32string translateString(std::wstring_view const inputString, TranslateStringOptions options) noexcept
{
    auto tmp = translateString<std::u16string>(inputString, options);
    return translateString<std::u32string>(tmp, options);
}

template<>
[[nodiscard]] inline std::u16string translateString(std::u32string_view const inputString, TranslateStringOptions options) noexcept
{
    std::u16string outputString;

    if (options._addBOM) {
        outputString.push_back(UNICODE_BOM);
    }

    for (auto inputCharacter: inputString) {
        if (inputCharacter >= UNICODE_Surrogates_BEGIN && inputCharacter <= UNICODE_Surrogates_END && !options._allowSurrogate) {
            inputCharacter = UNICODE_Replacement_Character;
        } else if (inputCharacter >= UNICODE_Plane_17_BEGIN) {
            inputCharacter = UNICODE_Replacement_Character;
        }

        if (inputCharacter >= UNICODE_Plane_1_BEGIN) {
            let surrogateCode = inputCharacter - UNICODE_Plane_1_BEGIN;
            let highSurrogate = UNICODE_High_Surrogates_BEGIN + (surrogateCode >> 10);
            let lowSurrogate = UNICODE_Low_Surrogates_BEGIN + (surrogateCode & 0x3ff);
            outputString.push_back(highSurrogate & 0xffff);
            outputString.push_back(lowSurrogate & 0xffff);

        } else {
            outputString.push_back(inputCharacter & 0xffff);
        }
    }

    return outputString;
}

template<>
[[nodiscard]] inline std::wstring translateString(std::u32string_view const inputString, TranslateStringOptions options) noexcept
{
    auto tmp = translateString<std::u16string>(inputString, options);
    return translateString<std::wstring>(tmp, options);
}

template<>
[[nodiscard]] inline std::string translateString(std::u32string_view const inputString, TranslateStringOptions options) noexcept
{
    std::string outputString;

    for (auto inputCharacter: inputString) {
        if (inputCharacter >= UNICODE_Surrogates_BEGIN && inputCharacter <= UNICODE_Surrogates_END && !options._allowSurrogate) {
            inputCharacter = UNICODE_Replacement_Character;
        } else if (inputCharacter >= UNICODE_Plane_17_BEGIN) {
            inputCharacter = UNICODE_Replacement_Character;
        }

        if (inputCharacter <= UNICODE_ASCII_END) {
            outputString.push_back(inputCharacter & 0x7f);

        } else if (inputCharacter <= 0x07ff) {
            outputString.push_back(((inputCharacter >> 6) & 0x1f) | 0xc0);
            outputString.push_back((inputCharacter & 0x3f) | 0x80);

        } else if (inputCharacter <= UNICODE_Basic_Multilinqual_Plane_END) {
            outputString.push_back(((inputCharacter >> 12) & 0x0f) | 0xe0);
            outputString.push_back(((inputCharacter >> 6) & 0x3f) | 0x80);
            outputString.push_back((inputCharacter & 0x3f) | 0x80);

        } else if (inputCharacter <= UNICODE_Plane_16_END) {
            outputString.push_back(((inputCharacter >> 18) & 0x07) | 0xf0);
            outputString.push_back(((inputCharacter >> 12) & 0x3f) | 0x80);
            outputString.push_back(((inputCharacter >> 6) & 0x3f) | 0x80);
            outputString.push_back((inputCharacter & 0x3f) | 0x80);
        }
    }

    return outputString;
}

[[nodiscard]] inline std::string to_string(std::wstring_view str) noexcept {
    return translateString<std::string>(str);
}

[[nodiscard]] inline std::string to_string(std::u16string_view str) noexcept {
    return translateString<std::string>(str);
}

[[nodiscard]] inline std::string to_string(std::u32string_view str) noexcept {
    return translateString<std::string>(str);
}

[[nodiscard]] inline std::wstring to_wstring(std::string_view str) noexcept {
    return translateString<std::wstring>(str);
}

[[nodiscard]] inline std::wstring to_wstring(std::u16string_view str) noexcept {
    return translateString<std::wstring>(str);
}

[[nodiscard]] inline std::wstring to_wstring(std::u32string_view str) noexcept {
    return translateString<std::wstring>(str);
}

[[nodiscard]] inline std::u16string to_u16string(std::string_view str) noexcept {
    return translateString<std::u16string>(str);
}

[[nodiscard]] inline std::u16string to_u16string(std::wstring_view str) noexcept {
    return translateString<std::u16string>(str);
}

[[nodiscard]] inline std::u16string to_u16string(std::u32string_view str) noexcept {
    return translateString<std::u16string>(str);
}

[[nodiscard]] inline std::u32string to_u32string(std::string_view str) noexcept {
    return translateString<std::u32string>(str);
}

[[nodiscard]] inline std::u32string to_u32string(std::wstring_view str) noexcept {
    return translateString<std::u32string>(str);
}

[[nodiscard]] inline std::u32string to_u32string(std::u16string_view str) noexcept {
    return translateString<std::u32string>(str);
}

[[nodiscard]] inline std::u32string splitLigature(char32_t x) noexcept
{
    switch (x) {
    case 0xfb00: return { 0x0066, 0x0066 }; // ff
    case 0xfb01: return { 0x0066, 0x0069 }; // fi
    case 0xfb02: return { 0x0066, 0x006c }; // fl
    case 0xfb03: return { 0x0066, 0x0066, 0x0069 }; // ffi
    case 0xfb04: return { 0x0066, 0x0066, 0x006c }; // ffl
    case 0xfb05: return { 0x017f, 0x0074 }; // long st
    case 0xfb06: return { 0x0073, 0x0074 }; // st

    case 0xfb13: return { 0x0574, 0x0576 }; // men now
    case 0xfb14: return { 0x0574, 0x0565 }; // men ech
    case 0xfb15: return { 0x0574, 0x056b }; // men ini
    case 0xfb16: return { 0x057e, 0x0576 }; // vew now
    case 0xfb17: return { 0x0574, 0x056d }; // men xeh

    default: return {};
    }
}

/*! Return line and column count at the end iterator.
 */
template<typename It>
[[nodiscard]] inline std::pair<int,int> count_line_and_columns(It begin, It const end)
{
    int line = 1;
    int column = 1;

    for (; begin != end; begin++) {
        switch (*begin) {
        case '\n': line++; [[fallthrough]];
        case '\r': column = 1;
            break;
        case '\t':
            column = ((((column-1) / 8) + 1) * 8) + 1;
            break;
        default:
            column++;
        }
    }
    return { line, column };
}

}
