
#pragma once

#include "ASTExpression.hpp"

namespace TTauri::Config {

struct ASTBoolean : ASTExpression {
    bool value;

    ASTBoolean(ASTLocation location, bool value) : ASTExpression(location), value(value) {}

    std::string str() override {
        return value ? "true" : "false";
    }

    Value execute(ExecutionContext *context) override { 
        return value;
    } 

};

}
