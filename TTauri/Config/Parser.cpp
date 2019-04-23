
#include "Parser.hpp"
#include "Token.hpp"

namespace TTauri::Config {

#define REQUIRE_TOKEN(x, c, i) if (c.isToken(i, x)) { i++ } else { return 0; }
#define REQUIRE_PARSE(x, c, i) if (auto tmpIndex = x(c, i)) { i = tmpIndex; } else { return 0; }

#define CHOICE_PARSE(x, c, i) if (auto tmpIndex = x(c, i)) { return i; }

size_t parseKeyStatement(ParseConrext &context, size_t tokenIndex)
{
    REQUIRE_TOKEN('[', context, tokenIndex)
    REQUIRE_PARSE(parseKey, context, tokenIndex)
    REQUIRE_TOKEN(']', context, tokenIndex)
    return tokenIndex; 
}

size_t parseStatement(ParseContext &context, size_t tokenIndex)
{
    CHOICE_PARSE(parseKeyStatement, context, tokenIndex)
    CHOICE_PARSE(parseAssignmentStatement, context, tokenIndex)
    CHOICE_PARSE(parseExpressionStatement, context, tokenIndex)
    return 0;
}

size_t parse(ParseContext &context, size_t tokenIndex)
{
    while (!context.isToken(tokenIndex, TokenType::EndOfFile)) {
        auto newTokenIndex = parseStatement(context, tokenIndex);
        if (newTokenIndex == tokenIndex) {
            return tokenIndex;
        }
        tokenIndex = newTokenIndex;
    }
    return tokenIndex;
}

}

