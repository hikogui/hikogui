// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "ASTExpression.hpp"

namespace TTauri::Config {

struct ASTBoolean : ASTExpression {
    bool value;

    ASTBoolean(Location location, bool value) : ASTExpression(location), value(value) {}

    std::string str() const override {
        return value ? "true" : "false";
    }

    Value execute(ExecutionContext *context) const override { 
        return value;
    } 

};

}
