// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Config/ASTExpression.hpp"

namespace TTauri::Config {

struct ASTRootObject : ASTExpression {

    ASTRootObject(Location location) noexcept : ASTExpression(location)  {}

    std::string string() const noexcept override {
        return "/";
    }

    datum &executeLValue(ExecutionContext &context) const override {
        return context.rootObject();
    } 
};

}
