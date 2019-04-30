// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "ASTExpression.hpp"

namespace TTauri::Config {

struct ASTAssignment : ASTExpression {
    ASTExpression *key;
    ASTExpression *expression;

    ASTAssignment(Location location, ASTExpression *key, ASTExpression *expression) : ASTExpression(location), key(key), expression(expression) {}
    ~ASTAssignment() {
        delete key;
        delete expression;
    }

    std::string str() const override {
        return key->str() + ":" + expression->str();
    }

    Value &executeLValue(ExecutionContext *context) const override {
        return key->executeAssignment(context, expression->execute(context));
    }

    void executeStatement(ExecutionContext *context) const override {
        execute(context);
    }
};

}
