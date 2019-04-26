
#pragma once

#include "ASTStatement.hpp"
#include "ASTExpression.hpp"

namespace TTauri::Config {

struct ASTExpressionStatement : ASTStatement {
    ASTExpression *expression;

    ASTExpressionStatement(ASTLocation location, ASTExpression *expression) : ASTStatement(location), expression(expression) {}
    ~ASTExpressionStatement() {
        delete expression;
    }

    std::string str() override {
        return expression->str();
    }
};

}
