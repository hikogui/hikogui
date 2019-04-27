
#pragma once

#include "ASTExpression.hpp"

namespace TTauri::Config {

struct ASTNull : ASTExpression {

    ASTNull(ASTLocation location) : ASTExpression(location) {}

    std::string str() override {
        return "null";
    }

    virtual std::shared_ptr<ValueBase> execute(ExecutionContext *context) override { 
        return std::make_shared<ValueNull>();
    } 

};

}
