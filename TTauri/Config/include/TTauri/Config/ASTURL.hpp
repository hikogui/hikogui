// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Config/ASTExpression.hpp"

namespace TTauri::Config {

struct ASTURL : ASTExpression {
    URL value;

    ASTURL(Location location, URL *value) noexcept : ASTExpression(location), value(*value) {
        delete value;
    }

    std::string string() const noexcept override {
        return to_string(value);
    }

    datum execute(ExecutionContext &context) const override {
        return datum{value};
    } 
};

}
