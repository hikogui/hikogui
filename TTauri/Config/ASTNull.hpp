// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "ASTExpression.hpp"

namespace TTauri::Config {

struct ASTNull : ASTExpression {

    ASTNull(Location location) noexcept : ASTExpression(location) {}

    std::string string() const noexcept override {
        return "null";
    }

    datum execute(ExecutionContext &context) const noexcept override { 
        return datum{datum::null{}};
    } 

};

}
