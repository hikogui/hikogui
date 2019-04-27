
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

    std::string str() override {
        return object->str() + "[" + index->str() + "]";
    }

    virtual std::shared_ptr<ValueBase> execute(ExecutionContext *context) override {
        return (*object->execute(context))[*index->execute(context)];
    }

    virtual std::shared_ptr<ValueBase> executeAssignment(ExecutionContext *context, const std::shared_ptr<ValueBase> &other) {
        (*object->execute(context))[*index->execute(context)] = other;
        return other;
    }
};

}