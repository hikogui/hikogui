
#include "Token.hpp"
#include <vector>
#include <functional>

namespace TTauri::Config {

static Token parseIntegerToken(const std::string &text, size_t offset)
{
    Token r = {TokenType::Integer, offset};

    bool startsWithZero = false;
    bool isNegative = false;
    int64_t value;
    int radix = 10;

    if (offset < text.size()) {
        switch (text[offset]) {
        case '+': isNegative = false; offset++; break;
        case '-': isNegative = true; offset++; break;
        } 
    }

    if (offset < text.size()) {
        if (text[offset] == '0') {
            startsWithZero = true;
            offset++;
        }
    }

    if (offset < text.size() && startsWithZero) {
        switch (text[offset]) {
        case 'b': case 'B': radix = 2; offset++; break;
        case 'o': case 'O': radix = 8; offset++; break;
        case 'd': case 'D': radix = 10; offset++; break;
        case 'x': case 'X': radix = 16; offset++; break;
        }
    }

    while (offset < text.size()) {
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

    return r(offset, isNegative ? -value : value);
}

static Token parseFloatToken(const std::string &text, size_t offset) {
    Token r = {TokenType::Float, offset};

    int64_t value = 0;
    int64_t fractional = 0;
    bool isNegative = false;

    if (offset < text.size()) {
        switch (text[offset]) {
        case '+': isNegative = false; offset++; break;
        case '-': isNegative = true; offset++; break;
        } 
    }

    while (offset < text.size()) {
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

    if (fractional == 0) {
        return r(TokenType::Invalid);
    } else {
        return r(offset, static_cast<double>(isNegative ? -value : value) / static_cast<double>(fractional));
    }
}

static Token parseStringToken(const std::string &text, size_t offset)
{
    Token r = {TokenType::String, offset};

    if (offset >= text.size() || text[offset] != '"') {
        return r(TokenType::Invalid);
    }
    offset++;

    std::string value;
    bool foundEscape = false;

    while (offset < text.size()) {
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
            return r(offset, value);
        } else {
            offset++;
            value.push_back(c);
        }
    }

    return r(TokenType::Invalid);
}

static Token parseCommentToken(const std::string &text, size_t offset)
{
    Token r = {TokenType::Comment, offset};

    if (offset + 1 >= text.size() || text[offset] != '/' || text[offset+1] != '/') {
        return r(TokenType::Invalid);
    }

    offset+= 2;

    while (offset < text.size()) {
        auto c = text[offset];
        if (c = '\n') {
            offset++;
            break;
        } else {
            offset++;
        }
    }

    return r(offset, {});
}

static Token parseIdentifierToken(const std::string &text, size_t offset) 
{
    Token r = {TokenType::Identifier, offset};

    std::string value;
    bool firstCharacter = true;

    while (offset < text.size()) {
        auto c = text[offset];

        if (
            c == '$' || c == '_' ||
            (c >= 'a' && c <='z') ||
            (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9' && !firstCharacter)
        ) {
            value.push_back(c);
            offset++;
        } else {
            break;
        }

        firstCharacter = false;
    }

    return r(offset, value);
}

static Token match(const std::string &haystack, size_t offset, const std::string &needle, TokenType tokenType, std::any value)
{
    if (offset + needle.size() > haystack.size()) {
        return {TokenType::Invalid, offset};
    }

    auto s = haystack.substr(offset, needle.size());
    return {tokenType, offset, needle.size(), std::move(value)};
}

static std::vector<std::string> binaryOperators = {
    "<<", ">>",
    "<=", ">=", "==", "!=", "<", ">",
    "or", "and", "xor",
    "+", "-", "/", "*", "%",
    "&", "|"
};

static Token parseRestOfTokens(const std::string &text, size_t offset) 
{
    Token r;

    if (offset == text.size()) {
        return {TokenType::EndOfFile, offset};
    }

    r = match(text, offset, "true", TokenType::Boolean, true);
    if (r.type != TokenType::Invalid) { return r; }

    r = match(text, offset, "false", TokenType::Boolean, false);
    if (r.type != TokenType::Invalid) { return r; }

    r = match(text, offset, "null", TokenType::Null, {});
    if (r.type != TokenType::Invalid) { return r; }

    for (auto x: binaryOperators) {
        r = match(text, offset, x, TokenType::BinaryOperator, x);
        if (r.type != TokenType::Invalid) { return r; }
    }

    for (auto x: std::vector<std::string>{ "not", "~" }) {
        r = match(text, offset, x, TokenType::UnaryOperator, x);
        if (r.type != TokenType::Invalid) { return r; }
    }

    switch (auto c = text[offset]) {
    case ':':
    case '=':
        return { TokenType::Assignment, offset, offset + 1 };
    case ',':
    case ';':
        return { TokenType::Terminator, offset, offset + 1 };
    case ' ':
    case '\n':
    case '\r':
    case '\f':
    case '\t':
        return { TokenType::Whitespace, offset, offset + 1 };
    case '{':
    case '}':
    case '[':
    case ']':
    case '(':
    case ')':
    case '.':
        return { TokenType::Character, offset, offset + 1, c };
    }

    return { TokenType::Invalid, offset };
}

std::vector<std::function<Token(const std::string &, size_t)>> tokenParsers = {
    parseIntegerToken,
    parseFloatToken,
    parseStringToken,
    parseCommentToken,
    parseIdentifierToken,
    parseRestOfTokens
};

Token parseToken(const std::string &text, size_t &offset)
{
    Token bestToken = {TokenType::Invalid, offset};

    for (const auto &tokenParser: tokenParsers) {
        auto token = tokenParser(text, offset);

        if (token.type == TokenType::Invalid) {
            continue;
        }

        if (token.length >= bestToken.length) {
            bestToken = token;
        }
    }

    offset = bestToken.offset + bestToken.length;
    return bestToken;
}

}