
#pragma once

#include "ASTExpression.hpp"

namespace TTauri::Config {

struct ASTName : ASTExpression {
    std::string name;

    ASTName(ASTLocation location, char *name) : ASTExpression(location), name(name) {
        free(name);
    }

    std::string str() override {
        return name;
    }

    virtual std::shared_ptr<ValueBase> execute(ExecutionContext *context) override {
        return (*context->objectStack.back())[name];
    } 

    virtual std::shared_ptr<ValueBase> executeAssignment(ExecutionContext *context, const std::shared_ptr<ValueBase> &other) {
        (*context->objectStack.back())[name] = other;
        return other;
    }
};

}
