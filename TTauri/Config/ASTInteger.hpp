// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "ASTExpression.hpp"
#include <string>

namespace TTauri::Config {

struct ASTInteger : ASTExpression {
    int64_t value;

    ASTInteger(Location location, int64_t value) noexcept : ASTExpression(location), value(value) {}

    std::string string() const noexcept override {
        return std::to_string(value);
    }

    universal_value execute(ExecutionContext &context) const noexcept override { 
        return value;
    } 

};

}
