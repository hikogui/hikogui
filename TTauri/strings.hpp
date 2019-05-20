// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include <string>
#include "utils.hpp"
#include <utf8proc/utf8proc.h>

namespace TTauri {

const char32_t UNICODE_Replacement_Character = 0xfffd;
const char32_t UNICODE_Surrogates_BEGIN = 0xd800;
const char32_t UNICODE_Surrogates_END = 0xdfff;
const char32_t UNICODE_High_Surrogates_BEGIN = 0xd800;
const char32_t UNICODE_High_Surrogates_END = 0xdbff;
const char32_t UNICODE_Low_Surrogates_BEGIN = 0xdc00;
const char32_t UNICODE_Low_Surrogates_END = 0xdfff;
const char32_t UNICODE_ASCII_END = 0x7f;
const char32_t UNICODE_Plane_0_END = 0xffff;
const char32_t UNICODE_Basic_Multilinqual_Plane_END = UNICODE_Plane_0_END;
const char32_t UNICODE_Plane_1_BEGIN = 0x010000;
const char32_t UNICODE_Plane_16_END = 0x10ffff;
const char32_t UNICODE_Plane_17_BEGIN = 0x110000;
const char32_t UNICODE_Zero_Width_No_Break_Space = 0xfeff;
const char32_t UNICODE_BOM = UNICODE_Zero_Width_No_Break_Space;
const char32_t UNICODE_Reverse_BOM = 0xfffe;

inline char32_t CP1252ToCodePoint(char inputCharacter)
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

    TranslateStringOptions &allowCP1252(bool allowCP1252_value = true) {
        _allowCP1252 = allowCP1252_value;
        return *this;
    }
    TranslateStringOptions &allowSurrogate(bool allowSurrogate_value = true) {
        _allowSurrogate = allowSurrogate_value;
        return *this;
    }
    TranslateStringOptions &byteSwap(bool byteSwap_value = true) {
        _byteSwap = byteSwap_value;
        return *this;
    }
    TranslateStringOptions &addBOM(bool addBOM_value = true) {
        _addBOM = addBOM_value;
        return *this;
    }
};

template<typename T, typename U>
inline T translateString(const U &inputString, TranslateStringOptions options = {})
{
    if constexpr (sizeof (T::value_type) == sizeof (U::value_type)) {
        return transform<T>(inputString, [](const U::value_type &inputCharacter) { return inputCharacter; });
    } else {
        const auto intermediateString = translateString<std::u32string>(inputString, options);
        return translateString<T>(intermediateString, options);
    }
}

template<>
inline std::u32string translateString(const std::string &inputString, TranslateStringOptions options)
{
    std::u32string outputString;
    char32_t codePoint = 0;
    size_t codePointToDo = 1;
    size_t backtrackPosition = 0;

    for (size_t i = 0; i < inputString.size(); i++) {
        auto inputCharacter = inputString.at(i);

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
                // UTF-16 bytecode mark should not apear in UTF-8.
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
inline std::u32string translateString(const std::u16string &inputString, TranslateStringOptions options)
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
inline std::u32string translateString(const std::wstring &inputString, TranslateStringOptions options)
{
    auto tmp = translateString<std::u16string>(inputString, options);
    return translateString<std::u32string>(tmp, options);
}

template<>
inline std::u16string translateString(const std::u32string &inputString, TranslateStringOptions options)
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
            auto const surrogateCode = inputCharacter - UNICODE_Plane_1_BEGIN;
            auto const highSurrogate = UNICODE_High_Surrogates_BEGIN + (surrogateCode >> 10);
            auto const lowSurrogate = UNICODE_Low_Surrogates_BEGIN + (surrogateCode & 0x3ff);
            outputString.push_back(highSurrogate & 0xffff);
            outputString.push_back(lowSurrogate & 0xffff);

        } else {
            outputString.push_back(inputCharacter & 0xffff);
        }
    }

    return outputString;
}

template<>
inline std::wstring translateString(const std::u32string &inputString, TranslateStringOptions options)
{
    auto tmp = translateString<std::u16string>(inputString, options);
    return translateString<std::wstring>(tmp, options);
}

template<>
inline std::string translateString(const std::u32string &inputString, TranslateStringOptions options)
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

inline std::string normalizeNFC(std::string str) {
    let s = utf8proc_NFC(reinterpret_cast<utf8proc_uint8_t const*>(str.data()));
    let r = std::string(reinterpret_cast<char *>(s));
    free(s);
    return r;
}

inline std::u32string splitLigature(char32_t x) {
    return {x};
}

inline std::u32string splitLigatures(std::u32string str) {
    std::u32string r;
    for (let character: str) {
        r += splitLigature(character);
    }
    return r;
}

}
