// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "ASTExpression.hpp"
#include "ASTExpressions.hpp"
#include "ASTMember.hpp"
#include "ASTName.hpp"
#include "TTauri/utils.hpp"
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
        auto const values = transform<std::vector<Value>>(arguments, [context](auto const x) {
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
                lv = result;
            } else {
                lv = v + result;
            }
        } catch (boost::exception &e) {
            e << boost::errinfo_file_name(location.file->string())
                << boost::errinfo_at_line(location.line)
                << errinfo_at_column(location.column);
            throw;
        }

    }
};

}
