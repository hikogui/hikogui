
#pragma once

#include "ASTExpression.hpp"

namespace TTauri::Config {

struct ASTSlice : ASTExpression {
    ASTExpression *object;
    std::vector<ASTExpression *> arguments;

    ASTSlice(ASTLocation location, ASTExpression *object, ASTExpressions *arguments) :
        ASTExpression(location),
        object(object),
        arguments(arguments->expressions)
    {
        // We copied the pointers of the expression, so they must not be destructed when expressions is deleted.
        arguments->expressions.clear();
        delete arguments;
    }

    ~ASTSlice() {
        delete object;
        for (auto argument: arguments) {
            delete argument;
        }
    }

    std::string str() override {
        std::string s = object->str() + "[";

        bool first = true;
        for (auto const argument: arguments) {
            if (!first) {
                s += ",";
            }
            s += argument->str();
            first = false;
        }

        return s + "]";
    }
};

}