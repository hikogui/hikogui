// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Config/ASTExpression.hpp"

namespace TTauri::Config {

struct ASTString : ASTExpression {
    std::string value;

    ASTString(Location location, char *value) noexcept : ASTExpression(location), value(value) {
        free(value);
    }

    std::string string() const noexcept override {
        return "\"" + value + "\"";
    }

    datum execute(ExecutionContext &context) const override {
        return datum{value};
    } 

    datum &executeAssignment(ExecutionContext &context, datum other) const override {
        try {
            auto &lv = context.currentObject()[value];
            lv = std::move(other);
            return lv;
        } catch (error &e) {
            e.set<"location"_tag>(location);
            throw;
        }
    }
};

}
