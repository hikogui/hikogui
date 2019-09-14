// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Config/LiteralParser.hpp"
#include <vector>
#include <functional>
#include <string>

namespace TTauri::Config {

int64_t parseInteger(const char *text, int radix, bool negative) noexcept
{
    required_assert(text);

    let size = strlen(text);
    size_t offset = 0;
    int64_t value = 0;

    while (offset < size) gsl_suppress(bounds.1) {
        let c = text[offset];
        if (c >= '0' && c <= '9') {
            value *= radix;
            value += (static_cast<int64_t>(c) - '0');
            offset++;
        } else if ((c >= 'a' && c <= 'f') || (c >= 'A' && c <='F')) {
            value *= radix;
            value += ((static_cast<int64_t>(c) | 0x20) - 'a' + 10);
            offset++;
        } else if (c == '_') {
            offset++;
        } else {
            break;
        }
    }

    return negative ? -value : value;
}

gsl_suppress(bounds.1)
double parseFloat(const char *text) noexcept {
    required_assert(text);

    let size = strlen(text);
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

    while (offset < size) gsl_suppress(bounds.1) {
        let c = text[offset];
        if (c >= '0' && c <= '9') {
            value *= 10;
            value += (static_cast<int64_t>(c) - '0');
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

    let fvalue = static_cast<double>(value) / static_cast<double>(fractional);
    let fvalue_ = isNegative ? -fvalue : fvalue;
    return fvalue_;
}

gsl_suppress2(f.6,r.11)
char *parseString(const char *text) noexcept
{
    required_assert(text);

    let size = strlen(text) - 1; // Ignore the last '"' or '>' character.
    size_t offset = 1; // Skip over first '"' or '<' character.
    std::string value;
    bool foundEscape = false;

    while (offset < size) gsl_suppress(bounds.1) {
        let c = text[offset];

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

    let r = strdup(value.data());
    required_assert(r);
    gsl_suppress2(lifetime.4,26487) return r;
}

URL *parseURL(const char *text)
{
    required_assert(text);

    auto s = std::string{text};
    if (s.front() == '<' && s.back() == '>') {
        s = s.substr(1, s.size() - 2);
    }

    gsl_suppress3(r.11,lifetime.4,26487) return new URL(s);
}

char *parseIdentifier(const char *text) noexcept 
{
    required_assert(text);

    return strdup(text);
}

bool parseBoolean(const char *text) noexcept
{
    required_assert(text);

    return strcmp(text, "true") == 0;
}

}
