
#pragma once

#include "ASTExpression.hpp"

namespace TTauri::Config {

struct ASTBoolean : ASTExpression {
    bool value;

    ASTBoolean(ASTLocation location, bool value) : ASTExpression(location), value(value) {}

    std::string str() override {
        return value ? "true" : "false";
    }

    virtual std::shared_ptr<ValueBase> execute(ExecutionContext *context) override { 
        return std::make_shared<ValueBoolean>(value);
    } 

};

}
