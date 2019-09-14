// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Config/ASTExpression.hpp"
#include <string>

namespace TTauri::Config {

struct ASTInteger : ASTExpression {
    int64_t value;

    ASTInteger(Location location, int64_t value) noexcept : ASTExpression(location), value(value) {}

    std::string string() const noexcept override {
        return std::to_string(value);
    }

    datum execute(ExecutionContext &context) const noexcept override { 
        return datum{value};
    } 

};

}
