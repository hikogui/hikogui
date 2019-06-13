// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "ASTExpression.hpp"
#include "ASTExpressionList.hpp"
#include "ASTMember.hpp"
#include "ASTName.hpp"
#include <vector>

namespace TTauri::Config {

struct ASTCall : ASTExpression {
    ASTExpression *object;
    std::vector<ASTExpression *> arguments;

    ASTCall(Location location, ASTExpression *object, ASTExpressionList *arguments) :
        ASTExpression(location),
        object(object),
        arguments(arguments->expressions)
    {
        // We copied the pointers of the expression, so they must not be destructed when expressions is deleted.
        arguments->expressions.clear();
        delete arguments;
    }

    ASTCall(Location location, ASTExpression *object, char *name, ASTExpressionList *arguments) :
        ASTExpression(location),
        object(new ASTMember(object->location, object, name)),
        arguments(arguments->expressions)
    {
        // We copied the pointers of the expression, so they must not be destructed when expressions is deleted.
        arguments->expressions.clear();
        delete arguments;
    }

    ASTCall(Location location, char *name, ASTExpressionList *arguments) :
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

    std::string string() const override {
        std::string s = object->string() + "(";

        bool first = true;
        for (let argument: arguments) {
            if (!first) {
                s += ",";
            }
            s += argument->string();
            first = false;
        }

        return s + ")";
    }

    Value execute(ExecutionContext *context) const override {
        let values = transform<std::vector<Value>>(arguments, [context](let x) {
            return x->execute(context);
        });

        return object->executeCall(context, values);
    } 

    void executeStatement(ExecutionContext *context) const override {
        auto result = execute(context);
        try {
            auto &lv = context->currentObject();
            auto v = lv;

            if (v.is_type<Undefined>()) {
                // An undefined value is replaced with the result of the call.
                // For example when including a file directly after a section.
                lv = result;
            } else {
                // If the value is already defined then try to add the result
                // of the call. For example including a file inside an object-literal.
                lv = v + result;
            }
        } catch (boost::exception &e) {
            e << errinfo_location(location);
            throw;
        }

    }
};

}
