// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "ASTExpression.hpp"

namespace TTauri::Config {

struct ASTVariableObject : ASTExpression {

    ASTVariableObject(Location location) noexcept : ASTExpression(location)  {}

    std::string string() const noexcept override {
        return "$";
    }

    universal_value &executeLValue(ExecutionContext *context) const override {
        return context->variableObject();
    } 
};

}
