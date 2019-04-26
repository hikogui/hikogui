
#pragma once

#include "ASTStatement.hpp"
#include "ASTExpression.hpp"

namespace TTauri::Config {

struct ASTAssignment : ASTStatement {
    ASTExpression *key;
    ASTExpression *expression;

    ASTAssignment(ASTLocation location, ASTExpression *key, ASTExpression *expression) : ASTStatement(location), key(key), expression(expression) {}
    ~ASTAssignment() {
        delete key;
        delete expression;
    }

    std::string str() override {
        return key->str() + ":" + expression->str();
    }
};

}
