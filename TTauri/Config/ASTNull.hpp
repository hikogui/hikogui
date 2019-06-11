// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "ASTExpression.hpp"

namespace TTauri::Config {

struct ASTNull : ASTExpression {

    ASTNull(Location location) : ASTExpression(location) {}

    std::string string() const override {
        return "null";
    }

    Value execute(ExecutionContext *context) const override { 
        return {{}};
    } 

};

}
