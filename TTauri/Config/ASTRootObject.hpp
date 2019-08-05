// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "ASTExpression.hpp"

namespace TTauri::Config {

struct ASTRootObject : ASTExpression {

    ASTRootObject(Location location) : ASTExpression(location)  {}

    std::string string() const override {
        return "/";
    }

    universal_value &executeLValue(ExecutionContext *context) const override {
        return context->rootObject();
    } 
};

}
