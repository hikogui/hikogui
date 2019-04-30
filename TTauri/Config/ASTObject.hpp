// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "ASTExpression.hpp"
#include "ASTExpressions.hpp"
#include <vector>
#include <map>

namespace TTauri::Config {

struct ASTObject : ASTExpression {
    std::vector<ASTExpression *> expressions;

    ASTObject(Location location) : ASTExpression(location), expressions() { }

    ASTObject(Location location, ASTExpression *expression) : ASTExpression(location), expressions({expression}) {
    }

    ASTObject(Location location, ASTExpressions *expressions) : ASTExpression(location), expressions(expressions->expressions) {
        // We copied the pointers of the expression, so they must not be destructed when expressions is deleted.
        expressions->expressions.clear();
        delete expressions;
    }

    ~ASTObject() {
        for (auto expression: expressions) {
            delete expression;
        }
    }

    std::string str() const override {
        std::string s = "{";

        bool first = true;
        for (auto const expression: expressions) {
            if (!first) {
                s += ",";
            }
            s += expression->str();
            first = false;
        }

        s += "}";
        return s;
    }

    Value execute(ExecutionContext *context) const override {
        context->pushObject();

        for (auto const expression: expressions) {
            expression->executeStatement(context);
        }

        return context->popObject();
    }

    Value execute() const {
        ExecutionContext context;
        return execute(&context);
    }

};

}
