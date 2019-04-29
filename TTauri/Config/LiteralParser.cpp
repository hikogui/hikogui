// Copyright 2019 Pokitec
// All rights reserved.

#include "LiteralParser.hpp"
#include <vector>
#include <functional>

namespace TTauri::Config {

int64_t parseInteger(const char *text, int radix, bool negative)
{
    size_t size = strlen(text);
    size_t offset = 0;
    int64_t value = 0;

    while (offset < size) {
        auto c = text[offset];
        if (c >= '0' && c <= '9') {
            value *= radix;
            value += (c - '0');
            offset++;
        } else if (c >= 'a' && c <= 'f' || c >= 'A' && c <='F') {
            value *= radix;
            value += ((c | 0x20) - 'a' + 10);
            offset++;
        } else if (c == '_') {
            offset++;
        } else {
            break;
        }
    }

    return negative ? -value : value;
}

double parseFloat(const char *text) {
    size_t size = strlen(text);
    size_t offset = 0;
    int64_t value = 0;
    int64_t fractional = 0;
    bool isNegative = false;

    if (offset < size) {
        switch (text[offset]) {
        case '+': isNegative = false; offset++; break;
        case '-': isNegative = true; offset++; break;
        } 
    }

    while (offset < size) {
        auto c = text[offset];
        if (c >= '0' && c <= '9') {
            value *= 10;
            value += (c - '0');
            offset++;
            if (fractional > 0) {
                fractional*= 10;
            }
        } else if (c == '_') {
            offset++;
        } else if (c == '.') {
            fractional = 1;
            offset++;
        } else {
            break;
        }
    }

    double fvalue = static_cast<double>(value) / static_cast<double>(fractional);
    double fvalue_ = isNegative ? -fvalue : fvalue;
    return fvalue_;
}

char *parseString(const char *text)
{
    size_t size = strlen(text) - 1; // Ignore the last '"' or '>' character.
    size_t offset = 1; // Skip over first '"' or '<' character.
    std::string value;
    bool foundEscape = false;

    while (offset < size) {
        auto c = text[offset];

        if (foundEscape) {
            foundEscape = false;
            switch (c) {
            case 'n': value.push_back('\n'); break;
            case 'r': value.push_back('\r'); break;
            case 't': value.push_back('\t'); break;
            case 'f': value.push_back('\f'); break;
            default: value.push_back(c);
            }

        } else if (c == '\\') {
            offset++;
            foundEscape = true;
        } else {
            offset++;
            value.push_back(c);
        }
    }

    return strdup(value.data());
}

char *parseIdentifier(const char *text) 
{
    return strdup(text);
}

bool parseBoolean(const char *text)
{
    return strcmp(text, "true") == 0;
}

}
