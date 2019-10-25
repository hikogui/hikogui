

#pragma once

#include <memory>
#include <string>
#include <string_view>

namespace TTauri {

struct Token {
    string_tag token;
    datum value;
    size_t offset;
};

template<typename TokenType, tag_string TokenTag>
Token<TokenType> parseToken(std::string_view text, size_t &offset)
{

}


}

