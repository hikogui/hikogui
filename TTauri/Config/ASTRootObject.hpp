// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "ASTExpression.hpp"

namespace TTauri::Config {

struct ASTRootObject : ASTExpression {

    ASTRootObject(Location location) : ASTExpression(location)  {}

    std::string str() const override {
        return "/";
    }

    Value &executeLValue(ExecutionContext *context) const override {
        return context->rootObject();
    } 
};

}
