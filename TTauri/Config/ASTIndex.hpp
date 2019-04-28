
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

    Value execute(ExecutionContext *context) override {
        return object->execute(context)[index->execute(context)];
    }

    Value executeAssignment(ExecutionContext *context, Value other) {
        object->execute(context)[index->execute(context)] = std::move(other);
        return other;
    }
};

}