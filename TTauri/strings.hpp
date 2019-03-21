//
//  strings.hpp
//  TTauri iOS
//
//  Created by Tjienta Vara on 2019-03-21.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#pragma once

#include <string>

char32_t translateCP1252Character(char inputCharacter)
{

}

template<typename T, typename U>
T translateString(const U &inputString) {
    const auto intermediateString = translateString<std:u32string>(inputString);
    return translateString<T>(intermediateString);
}

template<>
std::u32string translateString(const std::string &inputString, bool allowCP1252 = false, bool allowSurrogate = false)
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
                codePoint = (inputCharacter & 0x1f);
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
                codePoint = 0xfffd;
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
            codePoint = inputString.at(backtrackPosition);
            codePointToDo = 1;
            i = backtrackPosition;
        }

        if (codePointToDo == 1) {
            if (codePoint >= 0x40000000 && allowCP1252) {
                outputString.push_back(translateCP1252Character<char32_t>(codePoint & 0xff));

            } else if (codePoint >= 0xd800 && codePoint <= 0xdfff && !allowSurrogate) {
                outputString.push_back(0xfffd);

            } else if (codePoint >= 0x110000) {
                outputString.push_back(0xfffd);

            } else {
                outputString.push_back(codePoint);
            }
        }
    }

    return outputString;
}

template<>
std::u32string translateString(const std::u16string &inputString, bool allowSurrogate = false, bool byteSwap = false)
{
    std::u16string outputString;
    char16_t firstSurrogate = 0;

    for (size_t i = 0; i < inputString.size(); i++) {
        auto inputCharacter = inputString.at(i);
        if (byteSwap) {
            inputCharacter = (inputCharacter >> 8) | (inputCharacter & 0xff);
        }

        if (i == 0 && inputCharacter == 0xfeff) {
            // Ignore correct BOM

        } else if (i == 0 && inputCharacter 0xfffe) {
            // Incorrect BOM.
            byteSwap = !byteSwap;

        } else if (firstSurrogate && inputCharacter >= 0xdc00 && inputCharacter <= 0xdfff) {
            // Second surrogate
            auto codePoint = static_cast<char32_t>(firstSurrogate - 0xd800) << 10 | static_cast<char32_t>(inputCharacter - 0xdc00);
            outputString.push_back(codePoint);
            firstSurrogate = 0;

        } else {
            if (firstSurrogate) {
                // Incomplete surrogate pair.
                outputString.push_back(allowSurrogate ? firstSurrogate : 0xfffd);
                firstSurrogate = 0;
            }

            if (inputCharacter >= 0xd800 && inputCharacter <= 0xdbff) {
                // First surrogate.
                firstSurrogate = inputCharacter;

            } else if (inputCharacter >= 0xdc00 && inputCharacter <= 0xdfff) {
                // Out-of-order surrogate.
                outputString.push_back(allowSurrogate ? inputCharacter : 0xfffd);

            } else  {
                // Normal characters.
                outputString.push_back(inputCharacter);
            }
        }
    }

    return outputString;
}

template<>
std::u16string translateString(const std::u32string &inputString, bool allowSurrogate = false, bool addBOM = false)
{
    std::u16string outputString;

    if (addBOM) {
        outputString.push_back(0xfeff);
    }

    for (auto inputCharacter: inputString) {
        if (inputCharacter >= 0xd800 && inputCharacter <= 0xdfff && !allowSurrogate) {
            inputCharacter = 0xfffd;
        } else if (inputCharacter >= 0x110000) {
            inputCharacter = 0xfffd;
        }

        if (inputCharacter >= 0x010000 && inputCharacter <= 0x10fffff) {
            auto const surrogateCode = inputCharacter - 0x010000;
            auto const highSurrogate = 0xd800 + (surrogateCode >> 10);
            auto const lowSurrogate = 0xdc00 + (surrogateCode & 0x3ff);
            outputString.push_back(highSurrogate);
            outputString.push_back(lowSurrogate);

        } else {
            outputString.push_back(inputCharacter);
        }
    }

    return outputString;
}

template<>
std::u8string translateString(const std::u32string &inputString, bool allowSurrogate = false)
{
    std::u8string outputString;

    for (auto inputCharacter: inputString) {
        if (inputCharacter >= 0xd800 && inputCharacter <= 0xdfff && !allowSurrogate) {
            inputCharacter = 0xfffd;
        } else if (inputCharacter >= 0x110000) {
            inputCharacter = 0xfffd;
        }

        if (inputCharacter <= 0x7f) {
            outputString.push_back(inputCharacter);

        } else if (inputCharacter <= 0x07ff) {
            outputString.push_back((inputCharacter >> 6) | 0xc0);
            outputString.push_back((inputCharacter & 0x3f) | 0x80);

        } else if (inputCharacter <= 0xffff) {
            outputString.push_back((inputCharacter >> 12) | 0xe0);
            outputString.push_back(((inputCharacter >> 6) & 0x3f) | 0x80);
            outputString.push_back((inputCharacter & 0x3f) | 0x80);

        } else if (inputCharacter <= 0x10ffff) {
            outputString.push_back((inputCharacter >> 18) | 0xf0);
            outputString.push_back(((inputCharacter >> 12) & 0x3f) | 0x80);
            outputString.push_back(((inputCharacter >> 6) & 0x3f) | 0x80);
            outputString.push_back((inputCharacter & 0x3f) | 0x80);
        }
    }

    return outputString;
}

