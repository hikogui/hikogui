
#pragma once

#include "ASTExpression.hpp"
#include "ASTExpressions.hpp"

namespace TTauri::Config {

struct ASTArray : ASTExpression {
    std::vector<ASTExpression *> expressions;

    ASTArray(ASTLocation location) : ASTExpression(location), expressions() { }

    ASTArray(ASTLocation location, ASTExpressions *expressions) : ASTExpression(location), expressions(expressions->expressions) {
        // We copied the pointers of the expression, so they must not be destructed when expressions is deleted.
        expressions->expressions.clear();
        delete expressions;
    }

    ~ASTArray() {
        for (auto expression: expressions) {
            delete expression;
        }
    }

    std::string str() override {
        std::string s = "[";

        bool first = true;
        for (auto expression: expressions) {
            if (!first) {
                s += ",";
            }
            s += expression->str();
            first = false;
        }

        s += "]";
        return s;
    }

    Value execute(ExecutionContext *context) override {
        Array values;

        for (auto const expression: expressions) {
            values.push_back(expression->execute(context));
        }
        return {values};
    } 

};

}
