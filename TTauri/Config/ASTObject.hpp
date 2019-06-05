// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "ASTExpression.hpp"
#include "ASTExpressionList.hpp"
#include <vector>
#include <map>

namespace TTauri::Config {

struct ASTObject : ASTExpression {
    std::vector<ASTExpression *> expressions;

    ASTObject(Location location) : ASTExpression(location), expressions() { }

    ASTObject(Location location, ASTExpressionList *expressionList) : ASTExpression(location), expressions() {
        for (auto expression: expressionList->expressions) {
            if (auto obj = dynamic_cast<ASTObject *>(expression)) {
                // An object will be merged with this.
                for (auto e: obj->expressions) {
                    expressions.push_back(e);
                }
                // We stole all expressions of obj, so make sure they are not deleted by the destructor of obj.
                obj->expressions.clear();
                delete obj;
            } else {
                expressions.push_back(expression);
            }
        }

        // We stole all expressions of expressions, so make sure they are not deleted by the destructor of expressions.
        expressionList->expressions.clear();
        delete expressionList;
    }

    ~ASTObject() {
        for (auto expression: expressions) {
            delete expression;
        }
    }

    std::string str() const override {
        std::string s = "{";

        bool first = true;
        for (let expression: expressions) {
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

        for (let expression: expressions) {
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
