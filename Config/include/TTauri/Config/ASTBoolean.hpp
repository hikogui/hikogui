// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Config/ASTExpression.hpp"

namespace TTauri::Config {

struct ASTBoolean : ASTExpression {
    bool value;

    ASTBoolean(Location location, bool value) noexcept : ASTExpression(location), value(value) {}

    std::string string() const noexcept override {
        return value ? "true" : "false";
    }

    datum execute(ExecutionContext &context) const noexcept override { 
        return datum{value};
    }
};

}
