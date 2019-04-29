
#pragma once

#include "ASTExpression.hpp"

namespace TTauri::Config {

struct ASTBoolean : ASTExpression {
    bool value;

    ASTBoolean(ASTLocation location, bool value) : ASTExpression(location), value(value) {}

    std::string str() const override {
        return value ? "true" : "false";
    }

    Value execute(ExecutionContext *context) const override { 
        return value;
    } 

};

}
