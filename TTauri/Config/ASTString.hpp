// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "ASTExpression.hpp"

namespace TTauri::Config {

struct ASTString : ASTExpression {
    std::string value;

    ASTString(Location location, char *value) noexcept : ASTExpression(location), value(value) {
        free(value);
    }

    std::string string() const noexcept override {
        return "\"" + value + "\"";
    }

    universal_value execute(ExecutionContext *context) const override {
        return value;
    } 

    universal_value &executeAssignment(ExecutionContext *context, universal_value other) const override {
        auto &lv = context->currentObject()[value];
        lv = std::move(other);
        return lv;
    }
};

}
