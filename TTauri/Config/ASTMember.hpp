
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

    std::string str() override {
        return object->str() + "." + name;
    }

    Value execute(ExecutionContext *context) override { 
        return object->execute(context)[name];
    } 

    Value executeAssignment(ExecutionContext *context, Value other) {
        object->execute(context)[name] = std::move(other);
        return other;
    }

};

}
