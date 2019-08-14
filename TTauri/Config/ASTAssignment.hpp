// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "ASTExpression.hpp"

namespace TTauri::Config {

struct ASTAssignment : ASTExpression {
    ASTExpression *key;
    ASTExpression *expression;

    ASTAssignment(Location location, ASTExpression *key, ASTExpression *expression) noexcept : ASTExpression(location), key(key), expression(expression) {}
    ~ASTAssignment() {
        delete key;
        delete expression;
    }

    std::string string() const noexcept override {
        return key->string() + ":" + expression->string();
    }

    universal_value &executeLValue(ExecutionContext &context) const override {
        let value = expression->execute(context);

        if (holds_alternative<Undefined>(value)) {
            TTAURI_THROW(invalid_operation_error("right hand side value of assignment is Undefined")
                << error_info<"location"_tag>(location)
            );
        }

        return key->executeAssignment(context, value);
    }

    void executeStatement(ExecutionContext &context) const override {
        // We are ignoring the return value here, not fast, but a lot simpler.
        static_cast<void>(execute(context));
    }
};

}
