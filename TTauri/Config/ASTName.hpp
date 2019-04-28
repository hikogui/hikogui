
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

    Value execute(ExecutionContext *context) override {
        return context->objectStack.back()[name];
    } 

    Value executeAssignment(ExecutionContext *context, Value other) override {
        context->objectStack.back()[name] = other;
        return other;
    }
};

}
