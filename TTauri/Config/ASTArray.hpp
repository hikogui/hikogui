// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "ASTExpression.hpp"
#include "ASTExpressions.hpp"

namespace TTauri::Config {

struct ASTArray : ASTExpression {
    std::vector<ASTExpression *> expressions;

    ASTArray(Location location) : ASTExpression(location), expressions() { }

    ASTArray(Location location, ASTExpressions *expressions) : ASTExpression(location), expressions(expressions->expressions) {
        // We copied the pointers of the expression, so they must not be destructed when expressions is deleted.
        expressions->expressions.clear();
        delete expressions;
    }

    ~ASTArray() {
        for (auto expression: expressions) {
            delete expression;
        }
    }

    std::string str() const override {
        std::string s = "[";

        bool first = true;
        for (auto expression: expressions) {
            if (!first) {
                s += ",";
            }
            s += expression->str();
            first = false;
        }

        s += "]";
        return s;
    }

    Value execute(ExecutionContext *context) const override {
        Array values;

        for (auto const expression: expressions) {
            values.push_back(expression->execute(context));
        }
        return {values};
    }

    void executeStatement(ExecutionContext *context) const override {
        if (expressions.size() == 0) {
            context->setSection(nullptr);

        } else if (expressions.size() == 1) {
            context->setSection(nullptr);
            auto &lv = expressions.at(0)->executeLValue(context);
            context->setSection(&lv);

        } else {
            BOOST_THROW_EXCEPTION(InvalidOperationError()
                << boost::errinfo_file_name(location.file->string())
                << boost::errinfo_at_line(location.line)
                << errinfo_at_column(location.column)
                << errinfo_message("syntax error, expected 0 or 1 expression in section statement")
            );
        }
    }

};

}
