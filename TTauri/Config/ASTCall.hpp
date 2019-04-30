// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "ASTExpression.hpp"
#include "ASTExpressions.hpp"
#include "ASTMember.hpp"
#include "ASTName.hpp"
#include <vector>

namespace TTauri::Config {

struct ASTCall : ASTExpression {
    ASTExpression *object;
    std::vector<ASTExpression *> arguments;

    ASTCall(Location location, ASTExpression *object, ASTExpressions *arguments) :
        ASTExpression(location),
        object(object),
        arguments(arguments->expressions)
    {
        // We copied the pointers of the expression, so they must not be destructed when expressions is deleted.
        arguments->expressions.clear();
        delete arguments;
    }

    ASTCall(Location location, ASTExpression *object, char *name, ASTExpressions *arguments) :
        ASTExpression(location),
        object(new ASTMember(object->location, object, name)),
        arguments(arguments->expressions)
    {
        // We copied the pointers of the expression, so they must not be destructed when expressions is deleted.
        arguments->expressions.clear();
        delete arguments;
    }

    ASTCall(Location location, char *name, ASTExpressions *arguments) :
        ASTExpression(location),
        object(new ASTName(location, name)),
        arguments(arguments->expressions)
    {
        // We copied the pointers of the expression, so they must not be destructed when expressions is deleted.
        arguments->expressions.clear();
        delete arguments;
    }

    ASTCall(Location location, ASTExpression *object, char *name, ASTExpression *argument) :
        ASTExpression(location),
        object(new ASTMember(object->location, object, name)),
        arguments({argument})
    {
    }

    ASTCall(Location location, ASTExpression *object, char *name) :
        ASTExpression(location),
        object(new ASTMember(object->location, object, name)),
        arguments({})
    {
    }

    ~ASTCall() {
        delete object;
        for (auto argument: arguments) {
            delete argument;
        }
    }

    std::string str() const override {
        std::string s = object->str() + "(";

        bool first = true;
        for (auto const argument: arguments) {
            if (!first) {
                s += ",";
            }
            s += argument->str();
            first = false;
        }

        return s + ")";
    }

    Value execute(ExecutionContext *context) const override { 
        BOOST_THROW_EXCEPTION(NotImplementedError());
    } 

};

}
