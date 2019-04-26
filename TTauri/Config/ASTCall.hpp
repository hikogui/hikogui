
#pragma once

#include "ASTExpression.hpp"
#include "ASTExpressions.hpp"
#include "ASTMember.hpp"
#include "ASTName.hpp"
#include <vector>

namespace TTauri::Config {

struct ASTCall : ASTExpression {
    ASTExpression *object;
    std::vector<ASTExpression *> arguments;

    ASTCall(ASTLocation location, ASTExpression *object, char *name, ASTExpressions *arguments) :
        ASTExpression(location),
        object(new ASTMember(object->location, object, name)),
        arguments(arguments->expressions)
    {
        // We copied the pointers of the expression, so they must not be destructed when expressions is deleted.
        arguments->expressions.clear();
        delete arguments;
    }

    ASTCall(ASTLocation location, char *name, ASTExpressions *arguments) :
        ASTExpression(location),
        object(new ASTName(location, name)),
        arguments(arguments->expressions)
    {
        // We copied the pointers of the expression, so they must not be destructed when expressions is deleted.
        arguments->expressions.clear();
        delete arguments;
    }

    ASTCall(ASTLocation location, ASTExpression *object, char *name, ASTExpression *argument) :
        ASTExpression(location),
        object(new ASTMember(object->location, object, name)),
        arguments({argument})
    {
    }

    ASTCall(ASTLocation location, ASTExpression *object, char *name) :
        ASTExpression(location),
        object(new ASTMember(object->location, object, name)),
        arguments({})
    {
    }

    ~ASTCall() {
        delete object;
        for (auto argument: arguments) {
            delete argument;
        }
    }

    std::string str() override {
        std::string s = object->str() + "(";

        bool first = true;
        for (const argument: arguments) {
            if (!first) {
                s += ",";
            }
            s += argument->str();
            first = false;
        }

        return s + ")"
    }
};

}
