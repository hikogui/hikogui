// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "ASTExpression.hpp"
#include <string>

namespace TTauri::Config {

struct ASTMember : ASTExpression {
    ASTExpression *object;
    std::string name;

    ASTMember(ASTLocation location, ASTExpression *object, char *name) : ASTExpression(location), object(object), name(name) {
        free(name);
    }

    ~ASTMember() {
        delete object;
    }

    std::string str() const override {
        return object->str() + "." + name;
    }

    Value &executeLValue(ExecutionContext *context) const override {
        return object->executeLValue(context)[name];
    }

    Value &executeAssignment(ExecutionContext *context, Value other) const override {
        auto &lv = object->executeLValue(context)[name];
        lv = std::move(other);
        return lv;
    }
};

}
