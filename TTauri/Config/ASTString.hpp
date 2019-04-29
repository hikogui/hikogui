// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "ASTExpression.hpp"

namespace TTauri::Config {

struct ASTString : ASTExpression {
    std::string value;

    ASTString(ASTLocation location, char *value) : ASTExpression(location), value(value) {
        free(value);
    }

    std::string str() const override {
        return "\"" + value + "\"";
    }

    Value execute(ExecutionContext *context) const override {
        return value;
    } 

};

}
