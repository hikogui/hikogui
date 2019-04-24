
#pragma once

#include "Token.hpp"
#include <vector>
#include <string>

namespace TTauri::Config {

struct ParseContext {

    std::vector<std::string> prefixKey;
    std::vector<Token> token;

    template<typename T>
    T tokenValue(size_t pos) const {
        return std::any_cast<char>(token.at(pos).value);
    }

    bool isToken(size_t pos, TokenType x) const {
        return token.at(pos).type == x;
    }

    bool isToken(size_t pos, char x) const {
        return token.at(pos).type == TokenType::Character && tokenValue<char>(pos) == x;
    }

    template<typename T>
    bool isTokenAndIncrement(size_t &pos, T x) const {
        auto r = isToken(pos, x);
        if (r) {
            pos++;
        }
        return r;
    }

};

}