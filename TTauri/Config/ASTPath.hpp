// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "ASTExpression.hpp"

namespace TTauri::Config {

struct ASTPath : ASTExpression {
    std::filesystem::path value;

    ASTPath(Location location, char *value) : ASTExpression(location), value(value) {
        free(value);
    }

    std::string str() const override {
        return "<" + value.string() + ">";
    }

    Value execute(ExecutionContext *context) const override {
        return value;
    } 

};

}
