// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "ASTExpression.hpp"

namespace TTauri::Config {

struct ASTURL : ASTExpression {
    URL value;

    ASTURL(Location location, URL *value) noexcept : ASTExpression(location), value(*value) {
        delete value;
    }

    std::string string() const noexcept override {
        return to_string(value);
    }

    universal_value execute(ExecutionContext *context) const override {
        return value;
    } 
};

}