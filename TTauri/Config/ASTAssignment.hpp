// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "ASTExpression.hpp"

namespace TTauri::Config {

struct ASTAssignment : ASTExpression {
    ASTExpression *key;
    ASTExpression *expression;

    ASTAssignment(Location location, ASTExpression *key, ASTExpression *expression) : ASTExpression(location), key(key), expression(expression) {}
    ~ASTAssignment() {
        delete key;
        delete expression;
    }

    std::string string() const override {
        return key->string() + ":" + expression->string();
    }

    Value &executeLValue(ExecutionContext *context) const override {
        let value = expression->execute(context);

        if (value.is_type<Undefined>()) {
            BOOST_THROW_EXCEPTION(InvalidOperationError("right hand side value of assignment is Undefined")
                << errinfo_location(location)
            );
        }

        return key->executeAssignment(context, value);
    }

    void executeStatement(ExecutionContext *context) const override {
        execute(context);
    }
};

}
