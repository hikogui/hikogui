
#include "LiteralParser.hpp"
#include <vector>
#include <functional>

namespace TTauri::Config {

int64_t parseInteger(const char *text)
{
    size_t size = strlen(text);
    size_t offset = 0;
    bool startsWithZero = false;
    bool isNegative = false;
    int64_t value;
    int radix = 10;

    if (offset < size) {
        switch (text[offset]) {
        case '+': isNegative = false; offset++; break;
        case '-': isNegative = true; offset++; break;
        } 
    }

    if (offset < size) {
        if (text[offset] == '0') {
            startsWithZero = true;
            offset++;
        }
    }

    if (offset < size && startsWithZero) {
        switch (text[offset]) {
        case 'b': case 'B': radix = 2; offset++; break;
        case 'o': case 'O': radix = 8; offset++; break;
        case 'd': case 'D': radix = 10; offset++; break;
        case 'x': case 'X': radix = 16; offset++; break;
        }
    }

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

    return isNegative ? -value : value;
}

double parseFloatToken(const char *text) {
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

    return static_cast<double>(isNegative ? -value : value) / static_cast<double>(fractional);
}

char *parseStringToken(const char *text)
{
    size_t size = strlen(text);
    size_t offset = 0;
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
        } else if (c == '"') {
            offset++;
            break;
        } else {
            offset++;
            value.push_back(c);
        }
    }

    return strdup(value.data());
}

char *parseIdentifierToken(const char *text) 
{
    return strdup(text);
}




}