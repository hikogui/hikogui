
#pragma once

#include "ASTExpression.hpp"

namespace TTauri::Config {

struct ASTAssignment : ASTExpression {
    ASTExpression *key;
    ASTExpression *expression;

    ASTAssignment(ASTLocation location, ASTExpression *key, ASTExpression *expression) : ASTExpression(location), key(key), expression(expression) {}
    ~ASTAssignment() {
        delete key;
        delete expression;
    }

    std::string str() override {
        return key->str() + ":" + expression->str();
    }

    virtual std::shared_ptr<ValueBase> execute(ExecutionContext *context) override {
        return key->executeAssignment(context, expression->execute(context));
    }

    void executeStatement(ExecutionContext *context) {
        execute(context);
    }

};

}
