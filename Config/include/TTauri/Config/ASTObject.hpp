// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Config/ASTExpression.hpp"
#include "TTauri/Config/ASTExpressionList.hpp"
#include <vector>
#include <map>

namespace TTauri::Config {

struct ASTObject : ASTExpression {
    std::vector<gsl::not_null<ASTExpression *>> expressions;

    ASTObject(Location location) noexcept : ASTExpression(location), expressions() { }

    gsl_suppress2(26489,lifetime.1)
    ASTObject(Location location, ASTExpressionList *expressionList) noexcept : ASTExpression(location), expressions() {
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

    ASTObject() = delete;
    ASTObject(ASTObject const &node) = delete;
    ASTObject(ASTObject &&node) = delete;
    ASTObject &operator=(ASTObject const &node) = delete;
    ASTObject &operator=(ASTObject &&node) = delete;

    std::string string() const noexcept override {
        std::string s = "{";

        bool first = true;
        for (let expression: expressions) {
            if (!first) {
                s += ",";
            }
            s += expression->string();
            first = false;
        }

        s += "}";
        return s;
    }

    datum execute(ExecutionContext &context) const override {
        context.pushObject();

        for (let expression: expressions) {
            expression->executeStatement(context);
        }

        return context.popObject();
    }

    datum execute() const {
        ExecutionContext context;
        return execute(context);
    }
};

inline std::string to_string(ASTObject const &obj) noexcept
{
    return obj.string();
}

}
