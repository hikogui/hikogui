
#include "Parser.hpp"
#include "ParseContext.hpp"
#include "Token.hpp"
#include "AST.hpp"

#include "TTauri/utils.hpp"

#include <vector>
#include <string>
#include <algorithm>
#include <unordered_map>

namespace TTauri::Config {

using namespace std;

#define START auto const start = pos;
#define FAIL pos = start; return {};
#define REQUIRE(x) if (!context.isTokenAndIncrement(pos, x)) { FAIL }
#define OPTIONAL(x) context.isTokenAndIncrement(pos, x);

//shared_ptr<ASTExpression> parseExpression(ParseContext &context, size_t &pos);

enum class RLTokenType {
    Value,
    Operator,
    Function,
    Open,
    Close,
    Comma
};

enum class RLAssociativity {
    LeftToRight,
    RightToLeft
};

unordered_map<string, pair<size_t, RLAssociativity>> operatorPrecedenceTable = {
    {"."s, {2, RLAssociativity::LeftToRight}},
    {"~"s, {3, RLAssociativity::RightToLeft}},
    {"not"s, {3, RLAssociativity::RightToLeft}},
    {"*"s, {5, RLAssociativity::LeftToRight}},
    {"/"s, {5, RLAssociativity::LeftToRight}},
    {"%"s, {5, RLAssociativity::LeftToRight}},
    {"+"s, {6, RLAssociativity::LeftToRight}},
    {"-"s, {6, RLAssociativity::LeftToRight}},
    {"<<"s, {7, RLAssociativity::LeftToRight}},
    {">>"s, {7, RLAssociativity::LeftToRight}},
    {"<"s, {9, RLAssociativity::LeftToRight}},
    {">"s, {9, RLAssociativity::LeftToRight}},
    {"<="s, {9, RLAssociativity::LeftToRight}},
    {">="s, {9, RLAssociativity::LeftToRight}},
    {"=="s, {10, RLAssociativity::LeftToRight}},
    {"!="s, {10, RLAssociativity::LeftToRight}},
    {"&"s, {11, RLAssociativity::LeftToRight}},
    {"^"s, {12, RLAssociativity::LeftToRight}},
    {"|"s, {13, RLAssociativity::LeftToRight}},
    {"and"s, {14, RLAssociativity::LeftToRight}},
    {"xor"s, {15, RLAssociativity::LeftToRight}},
    {"or"s, {16, RLAssociativity::LeftToRight}}
};

struct RLToken {
    RLTokenType type;
    any value;
    size_t nrArguments;

    size_t precedence() const {
        if (type == RLTokenType::Operator) {
            const auto i = operatorPrecedenceTable.find(any_cast<string>(value));
            if (i != operatorPrecedenceTable.end()) {
                return (*i).second.first;
            } else {
                abort();
            }
        } else {
            abort();
        }
    }

    RLAssociativity associativity() const {
        if (type == RLTokenType::Operator) {
            const auto i = operatorPrecedenceTable.find(any_cast<string>(value));
            if (i != operatorPrecedenceTable.end()) {
                return (*i).second.second;
            } else {
                abort();
            }
        } else {
            abort();
        }
    }
};

shared_ptr<ASTExpression> shuntYard(vector<RLToken> input)
{
    vector<RLToken> stack;
    vector<RLToken> output;

    reverse(input.begin(), input.end());

    auto previousTokenType = RLTokenType::Operator;
    while (input.size()) {
        auto const token = pop_back(input);

        switch (token.type) {
        case RLTokenType::Value:
            output.push_back(token);
            break;
        
        case RLTokenType::Open:
            if (previousTokenType == RLTokenType::Operator) {
                stack.push_back(token);
            } else {
                stack.push_back({ RLTokenType::Function, {}, 0 });
            }
            break;

        case RLTokenType::Close:
            while (
                stack.size() &&
                stack.back().type != RLTokenType::Open &&
                stack.back().type != RLTokenType::Function
            ) {
                output.push_back(pop_back(stack));
            }

            if (stack.size()) {
                switch (stack.back().type) {
                case RLTokenType::Open:
                    stack.pop_back();
                    break;

                case RLTokenType::Function:
                    output.push_back(pop_back(stack));
                    break;
                }
            } else {
                abort();
                return {};
            }
            break;

        case RLTokenType::Comma:
            while (stack.size() && (stack.back().type != RLTokenType::Function)) {
                output.push_back(pop_back(stack));
            }

            // Record number of arguments passed to the function.
            stack.back().nrArguments++;
            stack.push_back(token);
            break;

        case RLTokenType::Operator:
            while (
                stack.size() && (
                    stack.back().precedence() > token.precedence() ||
                    (stack.back().precedence() == token.precedence() && stack.back().associativity() != RLAssociativity::LeftToRight)
                ) &&
                stack.back().type != RLTokenType::Open &&
                stack.back().type != RLTokenType::Function

            ) {
                output.push_back(pop_back(stack));
            }
        
            stack.push_back(token);
            break;
        }

        previousTokenType = token.type;
    }

    while (stack.size()) {
        auto const token = pop_back(stack);

        if (token.type == RLTokenType::Open || token.type == RLTokenType::Function) {
            abort();
        }
        output.push_back(token);
    }

}

shared_ptr<ASTExpression> parseExpression(ParseContext &context, size_t &pos)
{
    START
    vector<RLToken> tokens;
    size_t parenDepth = 0;

    while (
        !(parenDepth == 0 && context.isToken(pos, TokenType::Terminator)) &&
        !context.isToken(pos, '}') &&
        !context.isToken(pos, ']')
    ) {
        if (auto e = parseObject(context, pos)) {
            tokens.push_back({ RLTokenType::Value, e });

        } else if (auto e = parseList(context, pos)) {
            tokens.push_back({ RLTokenType::Value, e });

        } else if (context.isTokenAndIncrement(pos, TokenType::Integer)) {
            auto const e = make_shared<ASTLiteral>(pos - 1, context.tokenValue<int64_t>(pos - 1));
            auto const e_ = dynamic_pointer_cast<ASTExpression>(e);
            tokens.push_back({ RLTokenType::Value, e_ });

        } else if (context.isTokenAndIncrement(pos, TokenType::Float)) {
            auto const e = make_shared<ASTLiteral>(pos - 1, context.tokenValue<double>(pos - 1));
            auto const e_ = dynamic_pointer_cast<ASTExpression>(e);
            tokens.push_back({ RLTokenType::Value, e_ });

        } else if (context.isTokenAndIncrement(pos, TokenType::String)) {
            auto const e = make_shared<ASTLiteral>(pos - 1, context.tokenValue<string>(pos - 1));
            auto const e_ = dynamic_pointer_cast<ASTExpression>(e);
            tokens.push_back({ RLTokenType::Value, e_ });

        } else if (context.isTokenAndIncrement(pos, TokenType::Boolean)) {
            auto const e = make_shared<ASTLiteral>(pos - 1, context.tokenValue<bool>(pos - 1));
            auto const e_ = dynamic_pointer_cast<ASTExpression>(e);
            tokens.push_back({ RLTokenType::Value, e_ });

        } else if (context.isTokenAndIncrement(pos, TokenType::Null)) {
            auto const e = make_shared<ASTLiteral>(pos - 1);
            auto const e_ = dynamic_pointer_cast<ASTExpression>(e);
            tokens.push_back({ RLTokenType::Value, e_ });

        } else if (context.isTokenAndIncrement(pos, TokenType::Identifier)) {
            auto const e = make_shared<ASTIdentifier>(pos - 1, context.tokenValue<string>(pos - 1));
            auto const e_ = dynamic_pointer_cast<ASTExpression>(e);
            tokens.push_back({ RLTokenType::Value, e_ });
        
        } else if (context.isTokenAndIncrement(pos, TokenType::Character)) {
            auto c = context.tokenValue<char>(pos - 1);
            switch (c) {
            case '.':
                tokens.push_back({ RLTokenType::Operator, "."s });
                break;
            case '(':
                tokens.push_back({ RLTokenType::Open});
                parenDepth++;
                break;
            case ')':
                tokens.push_back({ RLTokenType::Close});
                parenDepth--;
                break;
            default:
                FAIL
            }

        } else if (context.isTokenAndIncrement(pos, TokenType::BinaryOperator)) {
            auto op = context.tokenValue<string>(pos - 1);
            tokens.push_back({ RLTokenType::Operator, op, 2});

        } else if (context.isTokenAndIncrement(pos, TokenType::Terminator)) {
            tokens.push_back({ RLTokenType::Comma });
       
        } else {
            FAIL
        }
    }

    return;
}


shared_ptr<ASTKey> parseKey(ParseContext &context, size_t &pos)
{
    START
        auto key = make_shared<ASTKey>(pos);

    do {
        if (context.isTokenAndIncrement(pos, TokenType::Identifier)) {
            auto id = make_shared<ASTIdentifier>(pos - 1, context.tokenValue<string>(pos - 1));
            key->ids.push_back(move(id));

        } else {
            FAIL
        }

    } while (context.isTokenAndIncrement(pos, '.'));

    return key;
}

shared_ptr<ASTKeyStatement> parseKeyStatement(ParseContext &context, size_t &pos)
{
    START
    auto keyStatement = make_shared<ASTKeyStatement>(pos);

    REQUIRE('[')

    if (context.isTokenAndIncrement(pos, ']')) {
        keyStatement->key = make_shared<ASTKey>(pos);
        return keyStatement;

    } else if (auto key = parseKey(context, pos)) {
        keyStatement->key = key;

        REQUIRE(']')
        return keyStatement; 

    } else {
        FAIL
    }
}

shared_ptr<ASTAssignmentStatement> parseAssignmentStatement(ParseContext &context, size_t &pos)
{
    START
    auto assignmentStatement = make_shared<ASTAssignmentStatement>(pos);

    if (auto key = parseKey(context, pos)) {
        assignmentStatement->key = key;
    } else {
        FAIL
    }

    REQUIRE(TokenType::Assignment)

    if (auto expression = parseExpression(context, pos)) {
        assignmentStatement->expression = expression;
    } else {
        FAIL
    }

    return assignmentStatement;
}

shared_ptr<ASTExpressionStatement> parseExpressionStatement(ParseContext &context, size_t &pos)
{
    START
    auto expressionStatement = make_shared<ASTExpressionStatement>(pos);

    if (auto expression = parseExpression(context, pos)) {
        expressionStatement->expression = expression;
    } else {
        FAIL
    }
    
    return expressionStatement;
}

shared_ptr<ASTStatement> parseStatement(ParseContext &context, size_t &pos)
{
    START

    if (auto p = parseKeyStatement(context, pos)) {
        return dynamic_pointer_cast<ASTStatement>(p);
    } else if (auto p = parseAssignmentStatement(context, pos)) {
        return dynamic_pointer_cast<ASTStatement>(p);
    } else if (auto p = parseExpressionStatement(context, pos)) {
        return dynamic_pointer_cast<ASTStatement>(p);
    } else {
        FAIL
    }
}

shared_ptr<ASTList> parseList(ParseContext &context, size_t &pos)
{
    START
    auto list = make_shared<ASTList>(pos);

    REQUIRE('[')

    while (!context.isToken(pos, ']')) {
        if (auto e = parseExpression(context, pos)) {
            list->expressions.push_back(move(e));
        } else {
            FAIL
        }

        if (!context.isTokenAndIncrement(pos, TokenType::Terminator)) {
            // No optional terminator, so we must be at end of list.
            break;
        }
    }

    REQUIRE(']')
    return list;
}

shared_ptr<ASTObject> parseObject(ParseContext &context, size_t &pos)
{
    START
    auto object = make_shared<ASTObject>(pos);

    REQUIRE('{')

    while (!context.isToken(pos, '}')) {
        bool requireTerminator = true;

        if (auto s = parseKeyStatement(context, pos)) {
            auto s_ = dynamic_pointer_cast<ASTStatement>(s);
            object->statements.push_back(move(s_));
            requireTerminator = false;

        } else if (auto s = parseAssignmentStatement(context, pos)) {
            auto s_ = dynamic_pointer_cast<ASTStatement>(s);
            object->statements.push_back(move(s_));
            requireTerminator = true;

        } else if (auto s = parseExpressionStatement(context, pos)) {
            auto s_ = dynamic_pointer_cast<ASTStatement>(s);
            object->statements.push_back(move(s_));
            requireTerminator = true;

        } else {
            FAIL
        }

        if (!context.isTokenAndIncrement(pos, TokenType::Terminator) && requireTerminator) {
            // No optional terminator, so we must be at end of list.
            break;
        }    
    }

    REQUIRE('}')
    return object;
}

shared_ptr<ASTObject> parse(ParseContext &context, size_t &pos)
{
    START
    auto object = make_shared<ASTObject>(pos);

    while (!context.isToken(pos, TokenType::EndOfFile)) {
        bool requireTerminator = true;

        if (auto s = parseKeyStatement(context, pos)) {
            auto s_ = dynamic_pointer_cast<ASTStatement>(s);
            object->statements.push_back(move(s_));
            requireTerminator = false;

        } else if (auto s = parseAssignmentStatement(context, pos)) {
            auto s_ = dynamic_pointer_cast<ASTStatement>(s);
            object->statements.push_back(move(s_));
            requireTerminator = true;

        } else if (auto s = parseExpressionStatement(context, pos)) {
            auto s_ = dynamic_pointer_cast<ASTStatement>(s);
            object->statements.push_back(move(s_));
            requireTerminator = true;

        } else {
            FAIL
        }

        if (requireTerminator) {
            REQUIRE(TokenType::Terminator)
        }
    }

    REQUIRE(TokenType::EndOfFile)
    return object;
}

}

