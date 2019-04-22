
#pragma once

#include "glm/glm.hpp"
#include <string>
#include <cstdint>
#include <any>

namespace TTauri::Config {

enum class TokenType {
    Invalid,
    Integer,
    Float,
    String,
    Identifier,
    Color,
    Boolean,
    Null,
    Comment,
    BinaryOperator,
    UnaryOperator,
    Assignment,
    Terminator,
    Whitespace,
    Character,
    EndOfFile
};

struct Token {
    TokenType type;
    size_t offset;
    size_t length = 0;
    std::any value = {};

    Token operator()(TokenType newType) const {
        return {newType, offset, length, value};
    }

    Token operator()(size_t endOffset, std::any newValue) const {
        if (endOffset - offset == 0) {
            return (*this)(TokenType::Invalid);
        } else {
            return {type, offset, endOffset - offset, std::move(newValue)};
        }
    }
};

Token parseToken(const std::string &text, size_t &offset);

}