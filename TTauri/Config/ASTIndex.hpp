// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "ASTExpression.hpp"

namespace TTauri::Config {

struct ASTIndex : ASTExpression {
    ASTExpression *object;
    ASTExpression *index;

    ASTIndex(ASTLocation location, ASTExpression *object, ASTExpression *index) : ASTExpression(location), object(object), index(index) {}

    ~ASTIndex() {
        delete object;
        delete index;
    }

    std::string str() const override {
        return object->str() + "[" + index->str() + "]";
    }

    Value &executeLValue(ExecutionContext *context) const override {
        return object->executeLValue(context)[index->execute(context)];
    }

    Value &executeAssignment(ExecutionContext *context, Value other) const override {
        auto &lv = object->executeLValue(context)[index->execute(context)];
        lv = std::move(other);
        return lv;
    }
};

}
